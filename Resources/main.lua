require "Common"

ROOT_FOLDER_ID = 0
TRASH_FOLDER_ID = 1
SEQUENCES_FOLDER_ID = 2

CHANNEL_EVENT_TYPE_NOP =                     0
CHANNEL_EVENT_TYPE_NOTE =                    1           -- 0x90, 0x80
CHANNEL_EVENT_TYPE_NOTE_OFF =                2           -- 0x80, reserved for conversion to studio sequence
CHANNEL_EVENT_TYPE_PROGRAM_CHANGE =          3           -- 0xC0
CHANNEL_EVENT_TYPE_MIXER_LEVEL_CHANGE =      4           -- 0xB0, CC: 0x07
CHANNEL_EVENT_TYPE_MIXER_BALANCE_CHANGE =    5           -- 0xB0, CC: 0x0A
CHANNEL_EVENT_TYPE_SUSTAIN =                 6           -- 0xB0, CC: 0x40
CHANNEL_EVENT_TYPE_PITCH_BEND =              7           -- 0xE0
CHANNEL_EVENT_TYPE_MODULATION =              8           -- 0xB0, CC: 0x01
CHANNEL_EVENT_TYPE_KEY_AFTERTOUCH =          9           -- 0xA0
CHANNEL_EVENT_TYPE_CHANNEL_AFTERTOUCH =      10          -- 0xD0
CHANNEL_EVENT_TYPE_CONTROL_CHANGE =          11          -- 0xB0
CHANNEL_EVENT_TYPE_SYSTEM_EXCLUSIVE =        12          -- 0xF0

-- META events

CHANNEL_EVENT_TYPE_META_SET_TEMPO =          0xF0       -- 0xFF, Meta: 0x51
CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE =     0xF1       -- 0xFF, Meta: 0x58
CHANNEL_EVENT_TYPE_META_END_OF_TRACK =       0xF2       -- 0xFF, Meta: 0x2F
CHANNEL_EVENT_TYPE_META_GENERIC =            0xFF       -- Generic Meta event not included in cases above


function addFolder()
    local folder = SequencesFolder.new()
    folder:setParentID(SEQUENCES_FOLDER_ID)
    sequencesDB:addFolder(folder)
end

function addTestSequence(name, isTempoDelayed, nbNotes, nbAnnotations)

    local sequence = MelobaseSequence.new()
    sequence:setName(name)
    local annotations = {}
    for i = 1,nbAnnotations do
        local annotation = SequenceAnnotation.new()
        annotation:setTickCount(1000 * i)
        table.insert(annotations, annotation)
    end
    sequence:setAnnotations(annotations)
    folder = sequencesDB:getFolderWithID(SEQUENCES_FOLDER_ID)
    sequence:setFolder(folder)

    local clip1 = sequence:tracks()[1]:clips()[1]

    local tsEvent = ChannelEvent.new()
    tsEvent:setType(CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE)
    tsEvent:setTickCount(0)
    tsEvent:setParam1(4)
    tsEvent:setParam2(4)
    tsEvent:setLength(0)
    clip1:addEvent(tsEvent)    
    
    local indexTempo = isTempoDelayed and 250 or 1

    local lastTickCount
    for i = 1,nbNotes do

        if i == indexTempo then
            local tEvent = ChannelEvent.new()
            tEvent:setType(CHANNEL_EVENT_TYPE_META_SET_TEMPO)
            lastTickCount = 120 * (i - 1)
            tEvent:setTickCount(lastTickCount)
            tEvent:setParam1(300000)
            tEvent:setParam2(0)
            tEvent:setLength(0)
            clip1:addEvent(tEvent)  
        end

        local event1 = ChannelEvent.new()
        event1:setType(CHANNEL_EVENT_TYPE_NOTE)
        lastTickCount = 120 * (i - 1);
        event1:setTickCount(lastTickCount)
        event1:setParam1(60 + (i - 1) % 10)
        event1:setParam2(80)
        event1:setLength(120)
        lastTickCount = lastTickCount + 120
        clip1:addEvent(event1)
    end

    local event1 = ChannelEvent.new()
    event1:setType(CHANNEL_EVENT_TYPE_META_END_OF_TRACK)
    event1:setTickCount(lastTickCount)
    clip1:addEvent(event1)

    sequencesDB:addSequence(sequence)

end

function addTestCC120Sequence()

    local sequence = MelobaseSequence.new()
    sequence:setName("Test CC120")
    folder = sequencesDB:getFolderWithID(SEQUENCES_FOLDER_ID)
    sequence:setFolder(folder)

    local clip1 = sequence:tracks()[1]:clips()[1]

    for i = 1,5 do

        local pcEvent = ChannelEvent.new()
        pcEvent:setType(CHANNEL_EVENT_TYPE_PROGRAM_CHANGE)
        pcEvent:setChannel(i - 1)
        pcEvent:setTickCount(0)
        pcEvent:setParam1(90)
        pcEvent:setParam2(0)
        pcEvent:setLength(0)
        clip1:addEvent(pcEvent)     
    
        local event1 = ChannelEvent.new()
        event1:setType(CHANNEL_EVENT_TYPE_NOTE)
        event1:setChannel(i - 1)
        event1:setTickCount(0)
        event1:setParam1(72 + (i - 1) % 10)
        event1:setParam2(80)
        event1:setLength(1500)
        clip1:addEvent(event1)

        local cc120Event = ChannelEvent.new()
        cc120Event:setType(CHANNEL_EVENT_TYPE_CONTROL_CHANGE)
        cc120Event:setChannel(i - 1)
        cc120Event:setTickCount(500)
        cc120Event:setParam1(120)
        cc120Event:setParam2(0)
        cc120Event:setLength(0)
        clip1:addEvent(cc120Event)
    
    end

    sequencesDB:addSequence(sequence)

end


function addTestCC123Sequence()

    local sequence = MelobaseSequence.new()
    sequence:setName("Test CC123")
    folder = sequencesDB:getFolderWithID(SEQUENCES_FOLDER_ID)
    sequence:setFolder(folder)

    local clip1 = sequence:tracks()[1]:clips()[1]

    for i = 1,5 do

        local pcEvent = ChannelEvent.new()
        pcEvent:setType(CHANNEL_EVENT_TYPE_PROGRAM_CHANGE)
        pcEvent:setChannel(i - 1)
        pcEvent:setTickCount(0)
        pcEvent:setParam1(90)
        pcEvent:setParam2(0)
        pcEvent:setLength(0)
        clip1:addEvent(pcEvent)     
    
        local event1 = ChannelEvent.new()
        event1:setType(CHANNEL_EVENT_TYPE_NOTE)
        event1:setChannel(i - 1)
        event1:setTickCount(0)
        event1:setParam1(72 + (i - 1) % 10)
        event1:setParam2(80)
        event1:setLength(1500)
        clip1:addEvent(event1)

        local cc123Event = ChannelEvent.new()
        cc123Event:setType(CHANNEL_EVENT_TYPE_CONTROL_CHANGE)
        cc123Event:setChannel(i - 1)
        cc123Event:setTickCount(500)
        cc123Event:setParam1(123)
        cc123Event:setParam2(0)
        cc123Event:setLength(0)
        clip1:addEvent(cc123Event)    
    
    end

    sequencesDB:addSequence(sequence)

end


function layout(sender)
    local r = sender:bounds()
    topView:setFrame(r)
end

top2View = getTopView()

undoManager = UndoManager.new()

studio = Studio.new(32, 64)
studio:startMixer()

sequencer = Sequencer.new(studio)
studioController = StudioController.new(studio, sequencer)
sequencesDB = SequencesDB.new(dataPath() .. "/melobase.sqlite", undoManager)
sequencesDB:open(false)

topView = TopView.new("topView", studio, 0.1, 14)

midiHub = MIDIHub.new()

server = Server.new(sequencesDB)

topViewController = TopViewController.new(topView, studioController, sequencesDB, server, midiHub:defaultMIDIInputPortName(), midiHub:defaultMIDIOutputPortName(), true)

midiHub:start(topViewController)

menuBar = getMenuBar()

top2View:addSubview(topView)

fileMenu = Menu.new("fileMenu", "File")
fileMenu:addMenuItem(MenuItem.new("MIDI Import..."))
fileMenu:addMenuItem(MenuItem.new("---"))
fileMenu:addMenuItem(MenuItem.new("MIDI Export..."))
fileMenu:addMenuItem(MenuItem.new("Audio Export..."))
fileMenu:addMenuItem(MenuItem.new("---"))
fileMenu:addMenuItem(MenuItem.new("Exit"))
fileMenu:addMenuItem(MenuItem.new("Power Off"))

fileMenu:setDidSelectItemFn(function(sender, itemIndex)
    if itemIndex == 6 then
        quit();
    elseif itemIndex == 7 then
        os.execute("poweroff");
        quit();
    end
    Menu.closePopUp(sender)
end)


editMenu = Menu.new("editMenu", "Edit")
editMenu:addMenuItem(MenuItem.new("Undo"))
editMenu:addMenuItem(MenuItem.new("Redo"))
editMenu:addMenuItem(MenuItem.new("---"))
editMenu:addMenuItem(MenuItem.new("Cut"))
editMenu:addMenuItem(MenuItem.new("Copy"))
editMenu:addMenuItem(MenuItem.new("Paste"))
editMenu:addMenuItem(MenuItem.new("---"))
editMenu:addMenuItem(MenuItem.new("Delete"))
editMenu:addMenuItem(MenuItem.new("---"))
editMenu:addMenuItem(MenuItem.new("Quantize"))
editMenu:addMenuItem(MenuItem.new("---"))
editMenu:addMenuItem(MenuItem.new("Select All"))

editMenu:setDidSelectItemFn(function(sender, itemIndex)
    if itemIndex == 1 then
        topViewController:undo()
    elseif itemIndex == 2 then
        topViewController:redo()
    elseif itemIndex == 4 then
        topViewController:editCut()
    elseif itemIndex == 5 then
        topViewController:editCopy()
    elseif itemIndex == 6 then
        topViewController:editPaste()
    elseif itemIndex == 8 then
        topViewController:editDelete()
    elseif itemIndex == 10 then
        topViewController:editQuantize()
    elseif itemIndex == 12 then
        topViewController:editSelectAll()
    end
    Menu.closePopUp(sender)
end)

isMetronomeSoundEnabled = false

controlMenu = Menu.new("controlMenu", "Control")
controlMenu:addMenuItem(MenuItem.new("Metronome Sound"))
controlMenu:addMenuItem(MenuItem.new("---"))
controlMenu:addMenuItem(MenuItem.new("Go to Beginning"))
controlMenu:addMenuItem(MenuItem.new("Play"))
controlMenu:addMenuItem(MenuItem.new("Record"))

controlMenu:setDidSelectItemFn(function(sender, itemIndex)
    if itemIndex == 1 then
        isMetronomeSoundEnabled = not isMetronomeSoundEnabled
        topViewController:setMetronomeSound(isMetronomeSoundEnabled)
    elseif itemIndex == 3 then
        topViewController:goToBeginning()
    elseif itemIndex == 4 then
        topViewController:playPause()
    elseif itemIndex == 5 then
        topViewController:record()
    end
    Menu.closePopUp(sender)
end)

sequencesMenu = Menu.new("sequencesMenu", "Sequences")
sequencesMenu:addMenuItem(MenuItem.new("New..."))
sequencesMenu:addMenuItem(MenuItem.new("New Subfolder"))
sequencesMenu:addMenuItem(MenuItem.new("---"))
sequencesMenu:addMenuItem(MenuItem.new("Convert to Multiple Tracks"))
sequencesMenu:addMenuItem(MenuItem.new("---"))
sequencesMenu:addMenuItem(MenuItem.new("Rate [0..5]"))
sequencesMenu:addMenuItem(MenuItem.new("Promote All"))
sequencesMenu:addMenuItem(MenuItem.new("Demote All"))
sequencesMenu:addMenuItem(MenuItem.new("Set As Played All"))
sequencesMenu:addMenuItem(MenuItem.new("Cleanup"))
sequencesMenu:addMenuItem(MenuItem.new("Empty Trash"))
sequencesMenu:addMenuItem(MenuItem.new("---"))
sequencesMenu:addMenuItem(MenuItem.new("Move to Trash"))

sequencesMenu:setDidSelectItemFn(function(sender, itemIndex)
    if itemIndex == 1 then
        topViewController:newSequence()
    elseif itemIndex == 2 then
        topViewController:newSubfolder()
    elseif itemIndex == 4 then
        topViewController:convertSequence()
    elseif itemIndex == 7 then
        topViewController:promoteAllSequences()
    elseif itemIndex == 8 then
        topViewController:demoteAllSequences()
    elseif itemIndex == 9 then
        topViewController:setAsPlayedAllSequences()
    elseif itemIndex == 10 then
        topViewController:cleanupSequences()
    elseif itemIndex == 11 then
        topViewController:emptyTrash()
    elseif itemIndex == 13 then
        topViewController:moveToTrashSequence()
    end
    Menu.closePopUp(sender)
end)

toolsMenu = Menu.new("toolsMenu", "Tools")
toolsMenu:addMenuItem(MenuItem.new("Options..."))

toolsMenu:setDidSelectItemFn(function(sender, itemIndex)
    if itemIndex == 1 then
        topViewController:showPreferences()
    end
    Menu.closePopUp(sender)
end)

windowMenu = Menu.new("windowMenu", "Window")
windowMenu:addMenuItem(MenuItem.new("Show Database"))
windowMenu:addMenuItem(MenuItem.new("Show Folders"))
windowMenu:addMenuItem(MenuItem.new("Show Utilities"))
windowMenu:addMenuItem(MenuItem.new("Show Controller Events"))
windowMenu:addMenuItem(MenuItem.new("Show Studio"))

windowMenu:setDidSelectItemFn(function(sender, itemIndex)
    if itemIndex == 1 then
        topViewController:showHideDatabase()
    elseif itemIndex == 2 then
        topViewController:showHideFolders()
    elseif itemIndex == 3 then
        topViewController:showHideProperties()
    elseif itemIndex == 4 then
        topViewController:showHideControllerEvents()
    elseif itemIndex == 5 then
        topViewController:showHideStudio()
    end
    Menu.closePopUp(sender)
end)


helpMenu = Menu.new("helpMenu", "Help")
helpMenu:addMenuItem(MenuItem.new("User Guide"))
helpMenu:addMenuItem(MenuItem.new("About Studio Engine..."))
helpMenu:addMenuItem(MenuItem.new("Check for Updates..."))

helpMenu:setDidSelectItemFn(function(sender, itemIndex)
    if itemIndex == 2 then
        topViewController:about()
    end
    Menu.closePopUp(sender)
end)

testsMenu = Menu.new("testsMenu", "Tests")
testsMenu:addMenuItem(MenuItem.new("Add test sequences"))

testsMenu:setDidSelectItemFn(function(sender, itemIndex)
    if itemIndex == 1 then
        addTestSequence("Tempo at beginning", false,500,0)
        addTestSequence("Tempo at middle", true,500,0)
        addTestSequence("With annotations", false,500, 10)
        addTestSequence("Huge sequence", false,120000, 10)
        addTestCC120Sequence()
        addTestCC123Sequence()
    end
    Menu.closePopUp(sender)
end)



menuBar:addMenu(fileMenu)
menuBar:addMenu(editMenu)
menuBar:addMenu(controlMenu)
menuBar:addMenu(sequencesMenu)
menuBar:addMenu(toolsMenu)
menuBar:addMenu(windowMenu)
menuBar:addMenu(helpMenu)
menuBar:addMenu(testsMenu)

top2View:setLayoutFn(layout)

server:start(8008)
