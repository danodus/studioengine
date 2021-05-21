//
//  dbviewcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-10-30.
//  Copyright (c) 2015-2021 Daniel Cliche. All rights reserved.
//

#include "dbviewcontroller.h"

#include <platform.h>

#include <algorithm>

#include "listitemview.h"
#include "sequencelistitemview.h"

// ---------------------------------------------------------------------------------------------------------------------
DBViewController::DBViewController(std::shared_ptr<DBView> view, MelobaseCore::SequencesDB* sequencesDB)
    : _view(view), _sequencesDB(sequencesDB) {
    using namespace std::placeholders;

    _sequenceSelectionDidChangeFn = nullptr;
    _folderSelectionDidChangeFn = nullptr;
    _foldersWillChangeFn = nullptr;
    _sequenceNameDidChangeFn = nullptr;
    _sequenceRatingDidChangeFn = nullptr;
    _focusDidChangeFn = nullptr;
    _didSetPaneVisibilityFn = nullptr;

    _sequencesFilter = MelobaseCore::SequencesDB::All;
    _sequencesOrderField = MelobaseCore::SequencesDB::Date;
    _sequencesOrderDirection = MelobaseCore::SequencesDB::Descending;

    _selectedFolder = nullptr;
    _isIncludingSubfolders = false;

    _isReceivingItems = false;
    _isAddingNewFolder = false;

    _statusFlagImage = std::make_shared<MDStudio::Image>("RedFlag@2x.png");
    _statusNewImage = std::make_shared<MDStudio::Image>("BlueDot@2x.png");
    _statusTrashImage = std::make_shared<MDStudio::Image>("TrashWhite@2x.png");

    _view->sequencesView()->tableView()->setNbColumnsFn(std::bind(&DBViewController::sequenceNbColumns, this, _1));
    _view->sequencesView()->tableView()->setColumnAtIndexFn(
        std::bind(&DBViewController::sequenceColumnAtIndex, this, _1, _2));
    _view->sequencesView()->tableView()->setNbRowsFn(std::bind(&DBViewController::nbSequences, this, _1));
    _view->sequencesView()->tableView()->setViewForRowFn(
        std::bind(&DBViewController::sequenceViewForRow, this, _1, _2));
    _view->sequencesView()->tableView()->setDidSelectRowFn(
        std::bind(&DBViewController::didSelectSequence, this, _1, _2));
    _view->sequencesView()->tableView()->setDidDeselectRowFn(
        std::bind(&DBViewController::didDeselectSequence, this, _1, _2));
    _view->sequencesView()->tableView()->setDidSetFocusStateFn(
        std::bind(&DBViewController::sequenceListDidSetFocusState, this, _1, _2));
    _view->sequencesView()->tableView()->setDidSetSortStateFn(
        std::bind(&DBViewController::tableViewDidSetSortState, this, _1, _2, _3));
    _view->sequencesView()->tableView()->setDidResizeColumnFn(
        std::bind(&DBViewController::tableViewDidResizeColumn, this, _1, _2, _3));

    _view->sequencesView()->showHideFoldersButton()->setStateDidChangeFn(
        std::bind(&DBViewController::showHideFoldersStateDidChange, this, _1, _2));

    _view->sequencesView()->filterSegmentedControl()->setSelectedSegment(0, false);

    _columnWidths = {20.0f, 140.0f, 90.0f, 200.0f};

    MDStudio::Platform::sharedInstance()->invoke([this]() { reloadSequences(); });

    _view->sequencesView()->filterSegmentedControl()->setDidSelectSegmentFn(
        std::bind(&DBViewController::filterChanged, this, _1, _2));
    _view->sequencesView()->nameSearchField()->setTextChangingFn(
        std::bind(&DBViewController::nameSearchFieldTextChanging, this, _1, _2));

    _view->foldersView()->treeView()->setNbRowsFn(std::bind(&DBViewController::nbFolders, this, _1, _2));
    _view->foldersView()->treeView()->setViewForIndexPathFn(
        std::bind(&DBViewController::folderViewForIndexPath, this, _1, _2, _3));
    _view->foldersView()->treeView()->setDidChangeRowExpandedState(
        std::bind(&DBViewController::didChangeFolderExpandedState, this, _1, _2, _3));
    _view->foldersView()->treeView()->setDidDeselectRowFn(
        std::bind(&DBViewController::didDeselectFolder, this, _1, _2));
    _view->foldersView()->treeView()->setDidSelectRowFn(std::bind(&DBViewController::didSelectFolder, this, _1, _2));
    _view->foldersView()->treeView()->setDidSetFocusStateFn(
        std::bind(&DBViewController::folderListDidSetFocusState, this, _1, _2));
    _view->foldersView()->treeView()->setDidPerformLayoutFn(
        std::bind(&DBViewController::folderTreeDidPerformLayout, this, _1));

    _view->foldersView()->newSubfolderButton()->setClickedFn(
        std::bind(&DBViewController::newSubfolderButtonClicked, this, _1));
    _view->foldersView()->recursiveButton()->setStateDidChangeFn(
        std::bind(&DBViewController::resursiveButtonStateDidChange, this, _1, _2));

    MDStudio::Platform::sharedInstance()->invoke([this]() { reloadFolders(); });

    // Initially, the left pane is not visible
    _view->splitView()->setLeftPaneVisibility(false);

    _view->foldersView()->newSubfolderButton()->setIsEnabled(false);
}

// ---------------------------------------------------------------------------------------------------------------------
DBViewController::~DBViewController() {}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int DBViewController::sequenceNbColumns(MDStudio::TableView* sender) { return 4; }

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::Column> DBViewController::sequenceColumnAtIndex(MDStudio::TableView* sender,
                                                                          unsigned int index) {
    MDStudio::Column::SortDirection columnSortDirection =
        _sequencesOrderDirection == MelobaseCore::SequencesDB::Ascending ? MDStudio::Column::Ascending
                                                                         : MDStudio::Column::Descending;

    switch (index) {
        case 0:
            return std::make_shared<MDStudio::Column>("", _columnWidths.at(index), false, false, false,
                                                      MDStudio::Column::Descending);
        case 1:
            return std::make_shared<MDStudio::Column>(
                _view->sequencesView()->ui().findString("dateAndTime"), _columnWidths.at(index), true, true,
                _sequencesOrderField == MelobaseCore::SequencesDB::Date, columnSortDirection);
        case 2:
            return std::make_shared<MDStudio::Column>(
                _view->sequencesView()->ui().findString("rating"), _columnWidths.at(index), false, true,
                _sequencesOrderField == MelobaseCore::SequencesDB::Rating, columnSortDirection);
        default:
            return std::make_shared<MDStudio::Column>(
                _view->sequencesView()->ui().findString("name"), _columnWidths.at(index), true, true,
                _sequencesOrderField == MelobaseCore::SequencesDB::Name, columnSortDirection);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int DBViewController::nbSequences(MDStudio::TableView* sender) {
    return static_cast<unsigned int>(
        _sequencesDB->getNbSequences(_sequencesFilter, sequencesNameSearch(), _selectedFolder, _isIncludingSubfolders));
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::View> DBViewController::sequenceViewForRow(MDStudio::TableView* sender, int row) {
    using namespace std::placeholders;

    auto view = std::shared_ptr<SequenceListItemView>(new SequenceListItemView(
        "sequenceListItemView" + std::to_string(row), nullptr, _columnWidths, _statusFlagImage, _statusNewImage,
        _statusTrashImage, _sequencesDB, row, _sequencesFilter, sequencesNameSearch(), _selectedFolder,
        _isIncludingSubfolders, _sequencesOrderField, _sequencesOrderDirection));
    view->setFocusState(sender->hasFocus());

    auto selectedRows = sender->selectedRows();
    if (std::find(selectedRows.begin(), selectedRows.end(), row) != selectedRows.end()) {
        view->setIsHighlighted(true);
        view->nameTextField()->setTextDidChangeFn(std::bind(&DBViewController::sequenceNameDidChange, this, _1, _2));
        view->ratingLevelIndicator()->setLevelDidChangeFn(
            std::bind(&DBViewController::sequenceRatingDidChange, this, _1, _2));
    }

    _view->topView()->registerForDragged(view);
    return view;
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::sequenceNameDidChange(MDStudio::TextField* sender, std::string text) {
    _sequenceNameDidChangeFn(this, text);
    MDStudio::Platform::sharedInstance()->invoke([=] {
        if (_view->sequencesView()->tableView()->isInChain() &&
            _view->sequencesView()->tableView()->responderChain()->capturedResponders().size() == 0)
            _view->sequencesView()->tableView()->captureFocus();
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::sequenceRatingDidChange(MDStudio::LevelIndicator* sender, float level) {
    _sequenceRatingDidChangeFn(this, level);
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::didSelectSequence(MDStudio::TableView* sender, int row) {
    using namespace std::placeholders;

    std::shared_ptr<MDStudio::View> view = sender->viewAtRow(row);
    std::shared_ptr<SequenceListItemView> sequenceListItemView = (std::static_pointer_cast<SequenceListItemView>)(view);
    sequenceListItemView->setIsHighlighted(true);
    sequenceListItemView->nameTextField()->setTextDidChangeFn(
        std::bind(&DBViewController::sequenceNameDidChange, this, _1, _2));
    sequenceListItemView->ratingLevelIndicator()->setLevelDidChangeFn(
        std::bind(&DBViewController::sequenceRatingDidChange, this, _1, _2));

    std::vector<std::shared_ptr<MelobaseCore::Sequence>> sequences;
    std::vector<int> rows = sender->selectedRows();

    _view->sequencesView()->tableView()->scrollToVisibleRectV(sequenceListItemView->rect());

    for (auto row : rows) {
        auto sequence =
            _sequencesDB->getSequence(row, _sequencesFilter, sequencesNameSearch(), _selectedFolder,
                                      _isIncludingSubfolders, _sequencesOrderField, _sequencesOrderDirection);
        sequences.push_back(sequence);
    }

    _sequenceSelectionDidChangeFn(this, sequences);
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::didDeselectSequence(MDStudio::TableView* sender, int row) {
    using namespace std::placeholders;

    std::shared_ptr<MDStudio::View> view = sender->viewAtRow(row);
    std::shared_ptr<SequenceListItemView> sequenceListItemView = (std::static_pointer_cast<SequenceListItemView>)(view);
    sequenceListItemView->setIsHighlighted(false);
    sequenceListItemView->nameTextField()->setTextDidChangeFn(nullptr);
    sequenceListItemView->ratingLevelIndicator()->setLevelDidChangeFn(nullptr);

    std::vector<std::shared_ptr<MelobaseCore::Sequence>> sequences;
    std::vector<int> rows = sender->selectedRows();

    for (auto row : rows) {
        auto sequence =
            _sequencesDB->getSequence(row, _sequencesFilter, sequencesNameSearch(), _selectedFolder,
                                      _isIncludingSubfolders, _sequencesOrderField, _sequencesOrderDirection);
        sequences.push_back(sequence);
    }

    _sequenceSelectionDidChangeFn(this, sequences);
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::sequenceListDidSetFocusState(MDStudio::TableView* sender, bool state) {
    for (auto view : sender->visibleRowViews()) {
        std::shared_ptr<SequenceListItemView> sequenceListItemView =
            (std::static_pointer_cast<SequenceListItemView>)(view);
        sequenceListItemView->setFocusState(state);
    }

    if (_focusDidChangeFn) _focusDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::tableViewDidSetSortState(MDStudio::TableView* sender, int columnIndex, bool state) {
    if (columnIndex == 1) {
        _sequencesOrderField = MelobaseCore::SequencesDB::Date;
    } else if (columnIndex == 2) {
        _sequencesOrderField = MelobaseCore::SequencesDB::Rating;
    } else if (columnIndex == 3) {
        _sequencesOrderField = MelobaseCore::SequencesDB::Name;
    }

    _sequencesOrderDirection = state ? MelobaseCore::SequencesDB::Ascending : MelobaseCore::SequencesDB::Descending;

    reloadSequences();
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::tableViewDidResizeColumn(MDStudio::TableView* sender, int columnIndex, float width) {
    _columnWidths.at(columnIndex) = width;
    for (auto rowView : sender->visibleRowViews()) {
        auto sequenceListItemView = std::dynamic_pointer_cast<SequenceListItemView>(rowView);
        sequenceListItemView->setColumnWidths(_columnWidths);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::showHideFoldersStateDidChange(MDStudio::Button* sender, bool state) {
    _view->splitView()->setLeftPaneVisibility(state);

    if (_didSetPaneVisibilityFn) _didSetPaneVisibilityFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::reloadSequences(bool isRowSelectionPreserved) {
    _sequencesDB->invalidateSequencesCache();

    // Check if we have at least one sequence in the database
    bool isSequenceAvailable = _sequencesDB->getNbSequences(MelobaseCore::SequencesDB::None, "", nullptr, false) > 0;
    _view->sequencesView()->noSequencesImageView()->setIsVisible(!isSequenceAvailable);
    _view->sequencesView()->tableView()->setIsVisible(isSequenceAvailable);
    _view->sequencesView()->filterSegmentedControl()->setIsVisible(isSequenceAvailable);
    _view->sequencesView()->nameSearchField()->setIsVisible(isSequenceAvailable);

    auto currentRows = _view->sequencesView()->tableView()->selectedRows();

    // Get the ID of the currently selected sequences
    auto selectedSequenceIDs = std::vector<UInt64>();

    if (isRowSelectionPreserved)
        for (auto currentRow : currentRows) {
            std::shared_ptr<MDStudio::View> selectedView = _view->sequencesView()->tableView()->viewAtRow(currentRow);
            std::shared_ptr<SequenceListItemView> selectedSequenceListItemView =
                (std::static_pointer_cast<SequenceListItemView>)(selectedView);
            selectedSequenceIDs.push_back(selectedSequenceListItemView->sequence()->id);
        }

    MDStudio::Point currentPos = _view->sequencesView()->tableView()->posInvY();

    _view->sequencesView()->tableView()->reload();

    if (isRowSelectionPreserved) {
        for (int row = 0; row < _view->sequencesView()->tableView()->nbRows(); ++row) {
            auto view = _view->sequencesView()->tableView()->viewAtRow(row);
            std::shared_ptr<SequenceListItemView> sequenceListItemView =
                (std::static_pointer_cast<SequenceListItemView>)(view);
            if (std::find(selectedSequenceIDs.begin(), selectedSequenceIDs.end(),
                          sequenceListItemView->sequence()->id) != selectedSequenceIDs.end())
                _view->sequencesView()->tableView()->setSelectedRow(row, true, false);
        }
        _view->sequencesView()->tableView()->setPosInvY(currentPos);
    } else {
        _view->sequencesView()->tableView()->setPosInvY(MDStudio::makePoint(0.0f, 0.0f));
    }
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<MelobaseCore::SequencesFolder>> DBViewController::getSubfolders(
    std::shared_ptr<MelobaseCore::SequencesFolder> parentFolder) {
    // Read all the subfolders
    std::vector<std::shared_ptr<MelobaseCore::SequencesFolder>> folders = _sequencesDB->getFolders(parentFolder);
    for (auto folder : folders) {
        if (folder->id == TRASH_FOLDER_ID) _trashFolder = folder;
        folder->parent = parentFolder;
        folder->subfolders = getSubfolders(folder);
    }

    return folders;
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::reloadFolders(bool isRowSelectionPreserved) {
    _sequencesDB->invalidateFoldersCache();

    // Create the hierarchy of folders
    _rootFolder = std::make_shared<MelobaseCore::SequencesFolder>();
    _rootFolder->subfolders = getSubfolders(_rootFolder);

    std::vector<int> currentRow = _view->foldersView()->treeView()->selectedRow();

    // Get the ID of the currently selected folder
    UInt64 selectedFolderID = 0;
    bool isDraftSelected = false;

    if (isRowSelectionPreserved && (currentRow.size() > 0)) {
        std::shared_ptr<MDStudio::View> selectedView = _view->foldersView()->treeView()->viewAtIndexPath(currentRow);
        std::shared_ptr<FolderListItemView> selectedFolderListItemView =
            (std::static_pointer_cast<FolderListItemView>)(selectedView);
        if (selectedFolderListItemView->folder()) {
            selectedFolderID = selectedFolderListItemView->folder()->id;
        } else {
            isDraftSelected = true;
        }
    }

    MDStudio::Point currentPos = _view->foldersView()->treeScrollView()->posInvY();

    _view->foldersView()->treeView()->reload();
    _view->foldersView()->treeScrollView()->setContentSize(_view->foldersView()->treeView()->contentSize());

    if (isRowSelectionPreserved) {
        if (isDraftSelected) {
            _view->foldersView()->treeView()->setSelectedRow({0});
        } else if (currentRow.size() > 0) {
            int row = 0;
            auto items = _view->foldersView()->treeView()->items();
            for (auto item : items) {
                if (row > 0) {
                    std::shared_ptr<FolderListItemView> folderListItemView =
                        (std::static_pointer_cast<FolderListItemView>)(item.second);
                    if (folderListItemView->folder()->id == selectedFolderID) {
                        _view->foldersView()->treeView()->setSelectedRow(item.first);
                        break;
                    }
                }
                row++;
            }
        }
        _view->foldersView()->treeView()->exposeRow(_view->foldersView()->treeView()->selectedRow());
        _view->foldersView()->treeScrollView()->setPosInvY(currentPos);
    } else {
        _view->foldersView()->treeScrollView()->setPosInvY(MDStudio::makePoint(0.0f, 0.0f));
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::filterChanged(MDStudio::SegmentedControl* sender, int selectedSegment) {
    switch (selectedSegment) {
        case 0:
            _sequencesFilter = MelobaseCore::SequencesDB::All;
            break;
        case 1:
            _sequencesFilter = MelobaseCore::SequencesDB::New;
            break;
        case 2:
            _sequencesFilter = MelobaseCore::SequencesDB::Annotated;
            break;
        case 3:
            _sequencesFilter = MelobaseCore::SequencesDB::Filter1;
            break;
        case 4:
            _sequencesFilter = MelobaseCore::SequencesDB::Filter2;
            break;
        case 5:
            _sequencesFilter = MelobaseCore::SequencesDB::Filter3;
            break;
        case 6:
            _sequencesFilter = MelobaseCore::SequencesDB::Filter4;
            break;
        case 7:
            _sequencesFilter = MelobaseCore::SequencesDB::Filter5;
            break;
    }
    reloadSequences(true);
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::nameSearchFieldTextChanging(MDStudio::SearchField* sender, std::string text) {
    reloadSequences(true);
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int DBViewController::nbFolders(MDStudio::TreeView* sender, std::vector<int> indexPath) {
    std::shared_ptr<MelobaseCore::SequencesFolder> folder = _rootFolder;

    for (auto index : indexPath) {
        folder = folder->subfolders[index];
    }

    return static_cast<unsigned int>(folder->subfolders.size());
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MelobaseCore::SequencesFolder> DBViewController::folderAtIndexPath(std::vector<int> indexPath,
                                                                                   bool* isExpanded) {
    std::shared_ptr<MelobaseCore::SequencesFolder> folder = _rootFolder;

    for (auto index : indexPath) {
        folder = folder->subfolders[index];
    }

    *isExpanded =
        std::find(_expandedFolderIDs.begin(), _expandedFolderIDs.end(), folder->id) != _expandedFolderIDs.end();

    return folder;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::View> DBViewController::folderViewForIndexPath(MDStudio::TreeView* sender,
                                                                         std::vector<int> indexPath, bool* isExpanded) {
    // Find the folder at the given index path
    std::shared_ptr<MelobaseCore::SequencesFolder> folder = folderAtIndexPath(indexPath, isExpanded);

    auto view = std::shared_ptr<FolderListItemView>(new FolderListItemView(
        "folderListItemView", nullptr, folder, (folder->id != TRASH_FOLDER_ID) && (folder->id != SEQUENCES_FOLDER_ID)));
    view->setFocusState(sender->hasFocus());

    using namespace std::placeholders;
    view->setDidReceiveSequenceFn(std::bind(&DBViewController::folderListItemViewDidReceiveSequence, this, _1, _2, _3));
    view->setDidReceiveFolderFn(std::bind(&DBViewController::folderListItemViewDidReceiveFolder, this, _1, _2, _3));

    if ((folder->id != TRASH_FOLDER_ID) && (folder->id != SEQUENCES_FOLDER_ID))
        _view->topView()->registerForDragged(view);

    return view;
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::folderNameDidChange(MDStudio::TextField* sender, std::string text) {
    _selectedFolder->name = text;

    // This callback is from a control present in the list that will be reloaded by the following call, so we invoke it
    MDStudio::Platform::sharedInstance()->invoke([=] { _sequencesDB->updateFolder(_selectedFolder); });
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::didChangeFolderExpandedState(MDStudio::TreeView* sender, std::vector<int> indexPath,
                                                    bool isExpanded) {
    bool isPreviouslyExpanded = false;
    auto folder = folderAtIndexPath(indexPath, &isPreviouslyExpanded);

    assert(folder);

    if (!isExpanded && isPreviouslyExpanded) {
        _expandedFolderIDs.erase(std::find(_expandedFolderIDs.begin(), _expandedFolderIDs.end(), folder->id));
    } else if (isExpanded && !isPreviouslyExpanded) {
        _expandedFolderIDs.push_back(folder->id);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::didDeselectFolder(MDStudio::TreeView* sender, std::vector<int> indexPath) {
    std::shared_ptr<MDStudio::View> view = sender->viewAtIndexPath(indexPath);
    std::shared_ptr<FolderListItemView> folderListItemView = (std::static_pointer_cast<FolderListItemView>)(view);
    folderListItemView->setIsHighlighted(false);
    folderListItemView->nameTextField()->setTextDidChangeFn(nullptr);
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::didSelectFolder(MDStudio::TreeView* sender, std::vector<int> indexPath) {
    using namespace std::placeholders;

    _selectedFolder = nullptr;

    std::shared_ptr<MDStudio::View> view = sender->viewAtIndexPath(indexPath);
    std::shared_ptr<FolderListItemView> folderListItemView = (std::static_pointer_cast<FolderListItemView>)(view);

    folderListItemView->setIsHighlighted(true);
    folderListItemView->nameTextField()->setTextDidChangeFn(
        std::bind(&DBViewController::folderNameDidChange, this, _1, _2));
    _view->foldersView()->treeScrollView()->scrollToVisibleRectV(folderListItemView->rect());

    _selectedFolder = folderListItemView->folder();

    _view->foldersView()->newSubfolderButton()->setIsEnabled(!isSelectedFolderTrash());

    if (_folderSelectionDidChangeFn) _folderSelectionDidChangeFn(this, _selectedFolder);
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::folderListDidSetFocusState(MDStudio::TreeView* sender, bool state) {
    auto items = sender->items();

    // Update the folder items
    for (unsigned int i = 0; i < items.size(); ++i) {
        std::shared_ptr<MDStudio::View> view = items[i].second;
        std::shared_ptr<FolderListItemView> folderListItemView = (std::static_pointer_cast<FolderListItemView>)(view);
        folderListItemView->setFocusState(state);
    }

    if (_focusDidChangeFn) _focusDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::folderTreeDidPerformLayout(MDStudio::TreeView* sender) {
    _view->foldersView()->treeScrollView()->setContentSize(sender->contentSize());
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::selectFolderWithID(UInt64 id, bool isEditionStarted) {
    // Select the added folder
    std::vector<std::pair<std::vector<int>, std::shared_ptr<MDStudio::View>>> items =
        _view->foldersView()->treeView()->items();

    for (auto item : items) {
        std::shared_ptr<MDStudio::View> view = item.second;
        std::shared_ptr<FolderListItemView> folderListItemView = (std::static_pointer_cast<FolderListItemView>)(view);
        if (folderListItemView->folder()->id == id) {
            _view->foldersView()->treeView()->exposeRow(item.first);
            _view->foldersView()->treeView()->setSelectedRow(item.first);
            if (isEditionStarted) {
                _view->foldersView()->treeView()->captureFocus();
                folderListItemView->nameTextField()->selectAllText();
                folderListItemView->updateResponderChain();
                folderListItemView->nameTextField()->startEdition();
            }
            break;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::newSubfolder() {
    if (_foldersWillChangeFn) _foldersWillChangeFn(this);

    // Make sure the folders view is visible
    if (!_view->sequencesView()->showHideFoldersButton()->state()) {
        _view->sequencesView()->showHideFoldersButton()->setState(true);
        _view->updateResponderChain();
    }

    auto folder = std::make_shared<MelobaseCore::SequencesFolder>();
    folder->date = folder->version = MDStudio::getTimestamp();
    folder->name = "New Folder";
    folder->rating = 0.0f;
    folder->parentID = _selectedFolder ? _selectedFolder->id : 0;
    folder->parent = _selectedFolder;
    _isAddingNewFolder = true;
    _sequencesDB->addFolder(folder);
    _isAddingNewFolder = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::newSubfolderButtonClicked(MDStudio::Button* sender) { newSubfolder(); }

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::resursiveButtonStateDidChange(MDStudio::Button* sender, bool state) {
    _isIncludingSubfolders = state;
    reloadSequences();
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::folderListItemViewDidReceiveSequence(FolderListItemView* sender,
                                                            std::shared_ptr<MelobaseCore::Sequence> sequence,
                                                            bool isLast) {
    static std::vector<std::shared_ptr<MelobaseCore::Sequence>> sequences;

    if (_foldersWillChangeFn) _foldersWillChangeFn(this);

    if (!_isReceivingItems) {
        _isReceivingItems = true;
        sequences.clear();
    }

    sequence->folder = sender->folder();
    sequences.push_back(sequence);

    if (isLast) {
        _sequencesDB->updateSequences(sequences);
        _isReceivingItems = false;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::folderListItemViewDidReceiveFolder(FolderListItemView* sender,
                                                          std::shared_ptr<MelobaseCore::SequencesFolder> folder,
                                                          bool isLast) {
    if (folder == sender->folder()) return;

    if (_foldersWillChangeFn) _foldersWillChangeFn(this);

    if (!_isReceivingItems) {
        _isReceivingItems = true;
        _sequencesDB->undoManager()->beginGroup();
    }

    // Remove from original folder
    auto originalFolder = folder->parent;
    originalFolder->subfolders.erase(
        std::find(originalFolder->subfolders.begin(), originalFolder->subfolders.end(), folder));

    // Set new parent
    folder->parentID = sender->folder()->id;
    folder->parent = sender->folder();
    sender->folder()->subfolders.push_back(folder);

    _sequencesDB->updateFolder(folder);

    if (isLast) {
        _sequencesDB->undoManager()->endGroup();
        _isReceivingItems = false;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void DBViewController::showHideFolders() {
    _view->sequencesView()->showHideFoldersButton()->setState(
        !_view->sequencesView()->showHideFoldersButton()->state());
}

// ---------------------------------------------------------------------------------------------------------------------
bool DBViewController::isSelectedFolderTrash() {
    if (_selectedFolder) {
        if (_selectedFolder->id == TRASH_FOLDER_ID) return true;

        // Check if the root folder is the trash
        std::shared_ptr<MelobaseCore::SequencesFolder> folder = _selectedFolder;
        while (folder->parent) {
            folder = folder->parent;
            if (folder->id == TRASH_FOLDER_ID) return true;
        }
    }
    return false;
}
