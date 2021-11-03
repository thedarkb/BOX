package require Tk
wm title . "Toolbox"
wm resizable . 0 0

set e 0
set tileNo 1
set sX 1
set sY 1
set layer 0
set tool 0

set toolLbl [label .labTool -text "Tool"]
set toolList [listbox .listTool -height 0 -exportselection 0]

set tileLbl [label .labTile -text "Tile"]
set tileSpin [spinbox .spinTile -from 1 -to 65535 -textvariable tileNo]

set layerLbl [label .labLayer -text "Layer"]
set layerList [listbox .listLayer -height 0 -exportselection 0]

set screenXLbl [label .labXscreen -text "X"]
set screenYLbl [label .labYscreen -text "Y"]
set screenXSpin [spinbox .spinXScreen -from 0 -to 65535 -textvariable sX]
set screenYSpin [spinbox .spinYScreen -from 0 -to 65535 -textvariable sY]

set selectedPixelXYlbl [label .labPxlXY -text "Selected Pixel X Y:"]
set selectedPixelXYtext [entry .textPxlXY]

set selectedTileXYlbl [label .labTleXY -text "Selected Tile X Y:"]
set selectedTileXYtext [entry .textTleXY]

set filenameLbl [label .labFile -text "Editing file:"]
set filenameText [entry .textFile]

.textPxlXY insert end "0 0"
.textTleXY insert end "0 0"
.textFile insert end ""
.listTool insert end Stamp Fill Select
.listLayer insert end Bottom Collision Top Entities All

grid $toolLbl $toolList  -padx 4 -pady 4 -sticky wn
grid $selectedPixelXYlbl $selectedPixelXYtext -padx 4 -pady 4 -sticky wn
grid $tileLbl $tileSpin  -padx 4 -pady 4 -sticky wn
grid $selectedTileXYlbl $selectedTileXYtext -padx 4 -pady 4 -sticky wn
grid $tileLbl $tileSpin -padx 4 -pady 4 -sticky wn
grid $layerLbl $layerList  -padx 4 -pady 4 -sticky wn
grid $filenameLbl $filenameText -padx 4 -pady 4 -sticky wn
grid $screenXLbl $screenXSpin  -padx 4 -pady 4 -sticky wn
grid $screenYLbl $screenYSpin -padx 4 -pady 4 -sticky wn
grid [button .brefresh -text "Reload Map Script" -command "internalRefresh"] [button .brebuild -text "Rebuild Map Script" -command "internalRebuild"]

wm protocol . WM_DELETE_WINDOW {

}

proc internalRefresh {} {
	refreshScript [.textFile get]
}

proc internalRebuild {} {
	buildScript [.textFile get]
}

proc listboxen {} {
	global layer
	global tool
	set layer [.listLayer curselection]
	set tool [.listTool curselection]
}