//
//  dbviewcontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-10-30.
//  Copyright (c) 2015-2021 Daniel Cliche. All rights reserved.
//

#ifndef DBVIEWCONTROLLER_H
#define DBVIEWCONTROLLER_H

#include <levelindicator.h>
#include <memory.h>
#include <searchfield.h>
#include <segmentedcontrol.h>
#include <sequencesdb.h>
#include <view.h>

#include "dbview.h"
#include "folderlistitemview.h"

class DBViewController {
   public:
    typedef std::function<void(DBViewController* sender,
                               std::vector<std::shared_ptr<MelobaseCore::Sequence>> sequences)>
        SequenceSelectionDidChangeFnType;
    typedef std::function<void(DBViewController* sender, std::shared_ptr<MelobaseCore::SequencesFolder> folder)>
        FolderSelectionDidChangeFnType;
    typedef std::function<void(DBViewController* sender)> FoldersWillChangeFnType;
    typedef std::function<void(DBViewController* sender, std::string name)> SequenceNameDidChangeFnType;
    typedef std::function<void(DBViewController* sender, float rating)> SequenceRatingDidChangeFnType;
    typedef std::function<void(DBViewController* sender)> FocusDidChangeFnType;
    typedef std::function<void(DBViewController* sender)> DidSetPaneVibilityFnType;

   private:
    std::shared_ptr<DBView> _view;

    MelobaseCore::SequencesDB* _sequencesDB;

    std::shared_ptr<MDStudio::Image> _statusFlagImage, _statusNewImage, _statusTrashImage;

    MelobaseCore::SequencesDB::sequencesFilterEnum _sequencesFilter;
    MelobaseCore::SequencesDB::sequencesOrderFieldEnum _sequencesOrderField;
    MelobaseCore::SequencesDB::orderDirectionEnum _sequencesOrderDirection;

    std::shared_ptr<MelobaseCore::SequencesFolder> _selectedFolder;
    bool _isIncludingSubfolders;

    std::shared_ptr<MelobaseCore::SequencesFolder> _rootFolder;
    std::shared_ptr<MelobaseCore::SequencesFolder> _trashFolder;

    std::vector<UInt64> _expandedFolderIDs;

    bool _isReceivingItems;
    bool _isAddingNewFolder;

    std::vector<float> _columnWidths;

    SequenceSelectionDidChangeFnType _sequenceSelectionDidChangeFn;
    FolderSelectionDidChangeFnType _folderSelectionDidChangeFn;
    FoldersWillChangeFnType _foldersWillChangeFn;
    SequenceNameDidChangeFnType _sequenceNameDidChangeFn;
    SequenceRatingDidChangeFnType _sequenceRatingDidChangeFn;
    FocusDidChangeFnType _focusDidChangeFn;
    DidSetPaneVibilityFnType _didSetPaneVisibilityFn;

    unsigned int sequenceNbColumns(MDStudio::TableView* sender);
    std::shared_ptr<MDStudio::Column> sequenceColumnAtIndex(MDStudio::TableView* sender, unsigned int index);
    unsigned int nbSequences(MDStudio::TableView* sender);
    std::shared_ptr<MDStudio::View> sequenceViewForRow(MDStudio::TableView* sender, int row);
    void sequenceNameDidChange(MDStudio::TextField* sender, std::string text);
    void sequenceRatingDidChange(MDStudio::LevelIndicator* sender, float level);
    void didSelectSequence(MDStudio::TableView* sender, int row);
    void didDeselectSequence(MDStudio::TableView* sender, int row);
    void sequenceListDidSetFocusState(MDStudio::TableView* sender, bool state);
    void tableViewDidSetSortState(MDStudio::TableView* sender, int columnIndex, bool state);
    void tableViewDidResizeColumn(MDStudio::TableView* sender, int columnIndex, float width);
    void showHideFoldersStateDidChange(MDStudio::Button* sender, bool state);

    void filterChanged(MDStudio::SegmentedControl* sender, int selectedSegment);
    void nameSearchFieldTextChanging(MDStudio::SearchField* sender, std::string text);

    void newSubfolderButtonClicked(MDStudio::Button* sender);
    void resursiveButtonStateDidChange(MDStudio::Button* sender, bool state);

    std::string sequencesNameSearch() { return _view->sequencesView()->nameSearchField()->text(); }

    unsigned int nbFolders(MDStudio::TreeView* sender, std::vector<int> indexPath);
    std::shared_ptr<MelobaseCore::SequencesFolder> folderAtIndexPath(std::vector<int> indexPath, bool* isExpanded);

    std::shared_ptr<MDStudio::View> folderViewForIndexPath(MDStudio::TreeView* sender, std::vector<int> indexPath,
                                                           bool* isExpanded);
    void folderNameDidChange(MDStudio::TextField* sender, std::string text);
    void didChangeFolderExpandedState(MDStudio::TreeView* sender, std::vector<int> indexPath, bool isExpanded);
    void didDeselectFolder(MDStudio::TreeView* sender, std::vector<int> indexPath);
    void didSelectFolder(MDStudio::TreeView* sender, std::vector<int> indexPath);
    void folderListDidSetFocusState(MDStudio::TreeView* sender, bool state);
    void folderTreeDidPerformLayout(MDStudio::TreeView* sender);

    void folderListItemViewDidReceiveSequence(FolderListItemView* sender,
                                              std::shared_ptr<MelobaseCore::Sequence> sequence, bool isLast);
    void folderListItemViewDidReceiveFolder(FolderListItemView* sender,
                                            std::shared_ptr<MelobaseCore::SequencesFolder> folder, bool isLast);

    std::vector<std::shared_ptr<MelobaseCore::SequencesFolder>> getSubfolders(
        std::shared_ptr<MelobaseCore::SequencesFolder> parentFolder);

   public:
    DBViewController(std::shared_ptr<DBView> view, MelobaseCore::SequencesDB* sequencesDB);
    ~DBViewController();

    std::shared_ptr<DBView> view() { return _view; }

    void reloadSequences(bool isRowSelectionPreserved = false);
    void reloadFolders(bool isRowSelectionPreserved = false);

    std::shared_ptr<MelobaseCore::SequencesFolder> selectedFolder() { return _selectedFolder; }
    bool isRecursive() { return _view->foldersView()->recursiveButton()->state(); }

    std::shared_ptr<MelobaseCore::SequencesFolder> trashFolder() { return _trashFolder; }

    void selectFolderWithID(UInt64 id, bool isEditionStarted = false);

    void showHideFolders();
    bool areFoldersVisible() { return _view->sequencesView()->showHideFoldersButton()->state(); }

    void newSubfolder();

    bool isSelectedFolderTrash();

    bool isAddingNewFolder() { return _isAddingNewFolder; }

    void setSequenceSelectionDidChangeFn(SequenceSelectionDidChangeFnType sequenceSelectionDidChangeFn) {
        _sequenceSelectionDidChangeFn = sequenceSelectionDidChangeFn;
    }
    void setFolderSelectionDidChangeFn(FolderSelectionDidChangeFnType folderSelectionDidChangeFn) {
        _folderSelectionDidChangeFn = folderSelectionDidChangeFn;
    }
    void setFoldersWillChangeFn(FoldersWillChangeFnType foldersWillChangeFn) {
        _foldersWillChangeFn = foldersWillChangeFn;
    }
    void setSequenceNameDidChangeFn(SequenceNameDidChangeFnType sequenceNameDidChangeFn) {
        _sequenceNameDidChangeFn = sequenceNameDidChangeFn;
    }
    void setSequenceRatingDidChangeFn(SequenceRatingDidChangeFnType sequenceRatingDidChangeFn) {
        _sequenceRatingDidChangeFn = sequenceRatingDidChangeFn;
    }
    void setFocusDidChangeFn(FocusDidChangeFnType focusDidChangeFn) { _focusDidChangeFn = focusDidChangeFn; }
    void setDidSetPaneVisibilityFn(DidSetPaneVibilityFnType didSetPaneVisibilityFn) {
        _didSetPaneVisibilityFn = didSetPaneVisibilityFn;
    }
};

#endif  // DBVIEWCONTROLLER_H
