global mcv_data_path = getINISetting ini_file "Paths" "data_path"
global subfileName = "-"

fn loadMCVScripts = (
	
	local ini_file = ( GetDir #scripts ) + "/mcv.ini"
	local src = getINISetting ini_file "Paths" "scripts_path"
	
	filein (src + "mcv_utils.ms")
	filein (src + "export_object.ms")
	filein (src + "export_scene.ms")
	filein (src + "export_model.ms")
	filein (src + "mcv_utility.ms")
	filein (src + "mcv_utils.ms")
	filein (src + "user_props.ms")
	
	format "MCV Scripts loaded\n"
)

loadMCVScripts()