utility MCVPanel "MCV" width:162 height:142
(

	button btn_export_scene "Export Scene" pos:[8,5] width:148 height:24 toolTip:"Export all objects as a new scene"
	edittext edt1 "Scene Name:" pos:[9,34] width:147 height:16
	edittext edt2 "         Subfile:" pos:[9,57] width:147 height:16
	button btn_path "..." pos:[5,77] width:150 height:24
	button btn_export_mesh "Export Mesh" pos:[5,105] width:150 height:24 toolTip:"Export selected objects as mesh"
	
	on MCVPanel open do
	(
		edt1.text = "level0"
		edt2.text = "-"
		-- Abrir el .ini y mirar cual es path
		local ini_file = ( GetDir #scripts ) + "/mcv.ini" 
		local out_path = getINISetting ini_file "Paths" "data_path"
		
		mcv_data_path = out_path
	
		btn_path.caption = out_path
		btn_path.toolTip = out_path
	)
		
	on btn_export_scene pressed do
	(
			try (
				local scene_name = edt1.text + ".xml"
				local subfile
				if(edt2.text == "-") then (subfile=scene_name)
				else (subfile=edt2.text)
				export_scene_to_file scene_name subfile
			) catch (
				messageBox (getCurrentException())
			)
		)
	on btn_export_mesh pressed do
	(
			try (
				exportMesh $
			) catch (
				messageBox (getCurrentException())
			)
		)
	--on edt1 entered text do
		--format "Exportando %\n" edt1.text
	on btn_path pressed do
	(
			local new_path = getSavePath caption:"Where is the data?" initialDir:(getDir #maxroot)
		format "New path is %\n" new_path
		--mcv_data_path = new_path

	)
)
