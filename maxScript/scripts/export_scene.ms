clearListener() 

function copyIfRequired the_bitmap &log_str = (
		-- Do we need to copy the file
	local src_file = the_bitmap.bitmap.filename 
	local dst_file = mcv_data_path + "textures\\" + (filenameFromPath src_file)
	local src_date = getFileModDate src_file
	
	local overwrite = false
 	if getFileSize dst_file == 0 then (
 		overwrite = false
 	) else (
-- 		local dst_date =getFileModDate dst_file
-- 		if src_date < dst_date then must_copy = true
		overwrite = true
	)
-- 		
 	if overwrite then (
		log_str = log_str + "- Overwriting "+ dst_file + " with " + src_file+"\n"
		deleteFile dst_file
 	)
	
	if copyFile src_file dst_file == false then (
			throw ("Can't copy texture file from " + src_file + " to " + dst_file )
	) else (
			log_str = log_str + "- Texture "+src_file+" copied to "+dst_file+"\n"
	)
)

function createDefaultMat &mat = (
	local newmat = StandardMaterial()
	newmat.name = ("default")
	--Relative path from the scripts folder
	local ini_file = ( GetDir #scripts ) + "/mcv.ini" 
	local src = getINISetting ini_file "Paths" "scripts_path"
	local defaultMatPath = src + "../../data/textures/hardcoded/"
	newmat.diffusemap = Bitmaptexture fileName: (defaultMatPath + "default.png")
	mat = newmat
)

function exportStdMtl obj mat &tempStream &log_str = (
	-- Convert obj to editable if it´s not already a mesh
	local meshObj
	if classof obj != Editable_Mesh then (
		meshObj = copy obj
		convertToMesh meshObj 
	)
	else meshObj = obj
	
	local channelA = 1
	local channelB = 3
	local channelC = 4
	
	if mat == undefined then createDefaultMat &mat
	if mat.diffusemap == undefined then createDefaultMat &mat
	if (classof mat.diffusemap == Checker) then createDefaultMat &mat --...
		
	format "			<mat>\n" to:tempStream
	local mat_name = getUserProp meshObj "material_name"
	if (mat_name == undefined) then (
		if (getUserProp meshObj "forward" == true) then mat_name = "tech_fwd_basic"
		else mat_name = "tech_basic"
	)
	--if (mat_name != undefined) then (
	--	format "				<material_name>%</material_name>\n" mat_name to:tempStream
	--)
	if (classof(mat.diffusemap)  == Mix) then
	(
		format "				<mix>\n" to:tempStream

		local has_lightmap = meshop.getMapSupport meshObj channelC
		if (has_lightmap == true) then 
		(
			mat_name = "tech_mix_lightmap"
		)
		else mat_name = "tech_mix"
		
		local map1_name = getFilenameFile mat.diffusemap.map1.bitmap.filename 
		format "					<diffuse>%</diffuse>\n" map1_name to:tempStream
		copyIfRequired mat.diffusemap.map1 &log_str
		local map2_name = getFilenameFile mat.diffusemap.map2.bitmap.filename 
		format "					<diffuse2>%</diffuse2>\n" map2_name to:tempStream
		copyIfRequired mat.diffusemap.map2 &log_str
		local mask_name = getFilenameFile mat.diffusemap.mask.bitmap.filename 
		format "					<mask>%</mask>\n" mask_name to:tempStream
		copyIfRequired mat.diffusemap.mask  &log_str
		format "				</mix>\n" to:tempStream
	)
	else if mat.diffusemap.bitmap != undefined then 
	(
		local bitmap_name = ""
		try
		(
			bitmap_name = getFilenameFile mat.diffusemap.bitmap.filename
		)
		catch ()
		if (bitmap_name != "") then 
		(
			format "				<diffuse>%</diffuse>\n" bitmap_name to:tempStream
			copyIfRequired mat.diffusemap  &log_str
		)
	)
	else 
	(
		createDefaultMat &mat
		local bitmap_name = ""
		try
		(
			bitmap_name = getFilenameFile mat.diffusemap.bitmap.filename
		)
		catch ()
		if (bitmap_name != "") then 
		(
			format "				<diffuse>%</diffuse>\n" bitmap_name to:tempStream
			copyIfRequired mat.diffusemap  &log_str
		)
	)

	if (mat.selfIllumMap != undefined) then (
		local selfIllumMap_name = ""
		try 
		(
			selfIllumMap_name = getFilenameFile mat.selfIllumMap.bitmap.filename 
		)
		catch()
		if (selfIllumMap_name != "") then (
			local has_mixmap = meshop.getMapSupport meshObj channelB
			if (has_mixmap == true) then 
			(
				mat_name = "tech_mix_lightmap"
			)
			else mat_name = "tech_lightmap"
			
			format "				<lightmap>%</lightmap>\n" selfIllumMap_name to:tempStream
			copyIfRequired mat.selfIllumMap  &log_str
		)
	)
	
	if mat.bumpMap != undefined then (
		local bumpmap_name = getFilenameFile mat.bumpMap.normal_map.filename 
		format "				<normalmap>%</normalmap>\n" bumpmap_name to:tempStream
		copyIfRequired mat.bumpmap.normal_map  &log_str
	)
	
	if mat.specularMap != undefined then (
		local specularMap_name = getFilenameFile mat.specularMap.bitmap.filename 
		format "				<specular>%</specular>\n" specularMap_name to:tempStream
		copyIfRequired mat.specularMap  &log_str
	)

	if mat.glossinessMap  != undefined then (
		local glossinessMap_name = getFilenameFile mat.glossinessMap.bitmap.filename 
		format "				<emissive>%</emissive>\n" glossinessMap_name to:tempStream
		copyIfRequired mat.glossinessMap  &log_str
	)
	
	if mat.displacementMap  != undefined then (
		local displacementMap_name = getFilenameFile mat.displacementMap.bitmap.filename 
		format "				<displacement>%</displacement>\n" displacementMap_name to:tempStream
		copyIfRequired mat.displacementMap  &log_str
	)
	format "				<material_name>%</material_name>\n" mat_name to:tempStream
	format "			</mat>\n" to:tempStream
	if meshObj != obj then delete meshObj
)

-- Exportar los materiales de esta malla
function exportMaterials obj mat_ids &tempStream &log_str = (
	
	local obj_mat = obj.material
	-- Si el objeto tiene un material std, no hacer caso de los material id's
	-- que pueda tener el objeto
	if classof obj_mat == Standardmaterial then (
		exportStdMtl obj obj_mat &tempStream &log_str
		
	) else if classof obj_mat == Multimaterial then (
		local idx = 1
		for mat_id in mat_ids do (		
			if mat_id != undefined then (		-- Significa que tenemos faces que usan ese slot
				local stdm = obj_mat[ idx ]
				if stdm == undefined then throw ("El multimaterial del obj " + obj.name + " no tiene la entrada " + ( idx as string) )
				if classof stdm != Standardmaterial then throw ("El multimaterial del obj " + obj.name + " no es de tipo std" )
				exportStdMtl obj stdm &tempStream &log_str
			)
			idx = idx + 1
		)
	) else if obj_mat == undefined then (
		log_str = log_str + ("- Object "+obj.name+" does not have material. Assigning default...\n")
		--Create newmaterial with name and texture by default
		createDefaultMat &obj.material
		obj_mat = obj.material
		
		--Export the standard mat
		exportStdMtl obj obj_mat &tempStream &log_str
		
	) else (
		-- El objeto tiene un material que es de tipo no soportado
		throw ("El obj " + obj.name + " usa un material no soportado: " + (classof obj_mat) )
	)
)

-- function exportUserProperties obj &fs =
-- (
-- 	local userProperties = filterstring (getUserPropBuffer obj) "\r\n"
-- 	if userProperties.count > 0 do
-- 	(
-- 		local tempStream = "" as stringstream
-- 		format "		<properties" to:tempStream
-- 		for i in userProperties do
-- 		(
-- 			format " %" i to:tempStream
-- 		)
-- 		format "/>\n" to: tempStream
-- 		
-- 		fs += tempStream
-- 	)
-- )

function export_model obj &tempStream &log_str = (
	log_str = log_str +("Exporting " + obj.name + "\n")
	
	-- Instancias  de este obj?
	local instances = #()
	instancemgr.GetInstances obj &instances
	
	-- Usar la primera instancia como nombre para la exportacion de la malla
	local mesh_name = instances[1].name
	
	-- La lista de caras de cada material usado
	local mat_ids = #()
	-- Exportar la malla en formato binario si soy la instancia
	-- q representa todas las instancias
	
	--format "		<model mesh='%'>\n" mesh_name to:tempStream
	format "		<model>\n" to:tempStream
	
	if (getUserProp obj "export_mesh" == true AND getUserProp obj "export_materials" == true) then
	(
		if instances[1] == obj then (
			mat_ids = exportMeshWithUVs obj &log_str
		)
		else log_str = log_str +("- Is instance\n")
		
		format "			<mesh>%</mesh>\n" mesh_name to:tempStream
		
		exportMaterials obj mat_ids &tempStream &log_str
	)
	else if (getUserProp obj "export_mesh" == true) then
	(
		if instances[1] == obj then (
			mat_ids = exportMesh obj &log_str
		)
		else log_str = log_str +("- Is instance\n")
		
		format "			<mesh>%</mesh>\n" mesh_name to:tempStream
	)
	
	-- will it be visible?
	local is_visible = getUserProp obj "is_visible"
	if (is_visible != undefined) then
		format "			<visible>%</visible>\n" is_visible to:tempStream
	else 
		format "			<visible>true</visible>\n" to:tempStream
	
	local max2local = inverse obj.transform
	local axis_x_90 = (quat 90 [1,0,0] ) as matrix3
	max2local = max2local * axis_x_90
	local minpos = obj.min
	local maxpos = obj.max
	minpos = minpos * max2local
	maxpos = maxpos * max2local
	
	format "			<aabb>% % % % % %</aabb>\n" minpos[1] minpos[2] minpos[3] maxpos[1] maxpos[2] maxpos[3] to:tempStream
	
	-- render flags?
	if(getUserProp obj "no_cull" != undefined) then (
		   local nocull = getUserProp obj "no_cull"
		   format "\t\t<no_cull>%</no_cull>\n" nocull to:tempStream
	)
	if(getUserProp obj "forward" != undefined) then (
		   local alphaName = getUserProp obj "forward"
		   format "\t\t<forward>%</forward>\n" alphaName to:tempStream
	)
	if(getUserProp obj "additive" != undefined) then (
		   local alphaName = getUserProp obj "additive"
		   format "\t\t<additive>%</additive>\n" alphaName to:tempStream
	)
	
	format "		</model>\n" to:tempStream
)

function export_physics obj &tempStream = (
	format "		<physics" to:tempStream
	
	if ((nvpx.IsConvex $) == true) then setUserProp obj "physics_shape" "CONVEX_MESH"

	local physics_shape = getUserProp obj "physics_shape"
	if physics_shape != undefined then 
		format " shape='%'" physics_shape to:tempStream
	else
		format " shape=CONCAVE_MESH" to:tempStream
		
	local physics_static = getUserProp obj "physics_static"
	local isStatic = false
	if physics_static != undefined then 
		if physics_static == true then isStatic = true
	
	format " static='%'" isStatic to:tempStream

	local physics_kinematic = getUserProp obj "physics_kinematic"
	local isKinematic = false
	if physics_kinematic != undefined then 
		if physics_kinematic == true then isKinematic = true
	
	format " kinematic='%'" isKinematic to:tempStream
	format "/>\n" to: tempStream
)

function export_player mtx_str &tempStream = (
	-- obj name
		format	"		<name>shadow</name>\n" to:tempStream
	-- obj type
		format	"		<type>PLAYER</type>\n" obj_type to:tempStream
	-- transform
		format	"		<transform>%</transform>\n" mtx_str to:tempStream
	-- export model component with mesh+materials
		format 	"		<model>\n"  to:tempStream
		format	"			<character>shadow</character>\n"  to:tempStream
		format	"			<mat>\n"  to:tempStream
		format	"				<material_name>tech_shadow</material_name>\n"  to:tempStream
		format	"				<diffuse>hardcoded/shadow_d</diffuse>\n"  to:tempStream
		format	"				<emissive>hardcoded/shadow_i</emissive>\n"  to:tempStream
		format	"				<normalmap>hardcoded/shadow_n</normalmap>\n"  to:tempStream
		format	"				<mask>hardcoded/shadow_bufanda</mask>\n"  to:tempStream
		format	"			</mat>\n"  to:tempStream
		format 	"			<visible>true</visible>\n" to:tempStream
		format	"			<aabb>-0.581745 -0.00498399 0.507839 0.409982 1.52444 -0.800522</aabb>\n"  to:tempStream
		format	"		</model>\n" to:tempStream
	-- export physics component
		format	"		<physics shape='CHARACTER' static='false' kinematic='false'/>\n" to:tempStream
	
	format "Player exported\n"
)

function export_trigger obj &tempStream = (
	local radius = obj.radius --OJO: se da por hecho que los triggers siempre serán esferas
	format "\t\t\t<trigger_radius>%</trigger_radius>\n" radius to:tempStream
	
	local trigger_type = getUserProp obj "trigger_type"
	format "\t\t\t<trigger_type>%</trigger_type>\n" trigger_type to:tempStream
	
	local command = getUserProp obj "command"
	format "\t\t\t<trigger_command>%</trigger_command>\n" command to:tempStream
)

function export_scene_obj obj &fs  &log_str = (
	-- comienza export de objeto
	local obj_type = getUserProp obj "obj_type"
	if (obj_type == undefined) then obj_type = "DEFAULT"
	--local info = "" as stringstream
	--format "Exporting object % of type %" obj.name obj_type to:info
	--print info
	
	-- Convertir la matrix de transformacion al espacio del mcv
	local mtx_max = obj.transform
	local max2mcv = (quat 90 [1,0,0] ) as matrix3
	local mcv2max = inverse max2mcv
	local mtx_mcv = mcv2max * mtx_max * max2mcv
	
	local mtx_str = matrix_as_string mtx_mcv
	
	-- string temporal para exportar info de objeto
	local tempStream = "" as stringstream
	
	-- start <obj> 
	format "	<obj>\n" to:tempStream
	
	if(obj.name == "player") then
		export_player mtx_str &tempStream
	else
	(
		-- obj name
		format "		<name>%</name>\n" \
				   obj.name to:tempStream
		
		-- obj type
		if (obj_type != undefined) then
		(
			format "		<type>%</type>\n" obj_type to:tempStream
			if(obj_type=="TRIGGER") then export_trigger obj &tempStream
		)
		else 
			format "		<type>DEFAULT</type>\n" to:tempStream
		
		-- transform del obj
		format "		<transform>%</transform>\n" \
						mtx_str to:tempStream
		

		
		-- export model component with mesh+materials
		if (getUserProp obj "export_mesh" == true) then (
			export_model obj &tempStream &log_str
		)
		
		-- export physics component
		if(getUserProp obj "export_physics" == true) then export_physics obj &tempStream
		
		--export collision type
		local col_type = getUserProp obj "col_type"
		if (col_type != undefined) then
			format "		<col_type>%</col_type>\n" col_type to:tempStream
		
		-- export other components
		
		if(getUserProp obj "sound" != undefined) then (
			   local soundName = getUserProp obj "sound"
			   format "\t\t<sound>%</sound>\n" soundName to:tempStream
		)
		
		if(getUserProp obj "steps" != undefined) then (
			   local soundName = getUserProp obj "steps"
			   format "\t\t<steps>%</steps>\n" soundName to:tempStream
		)
		
		if(getUserProp obj "billboard" != undefined) then (
			   local billboardName = getUserProp obj "billboard"
			   format "\t\t<billboard>%</billboard>\n" billboardName to:tempStream
		)
		
		if(getUserProp obj "particles" != undefined) then (
			   local particlesName = getUserProp obj "particles"
			   format "\t\t<particles>%</particles>\n" particlesName to:tempStream
		)
		
		if(getUserProp obj "disabled" != undefined) then (
			   format "\t\t<disabled/>\n" billboardName to:tempStream
		)

		
		--exportUserProperties obj &tempStream
	)
	
	-- Cerrar el xml del objeto
	format "	</obj>\n" to:tempStream
	
	fs += tempStream
)

-- Exportar todos los objectos en xml al file stream
function export_scene &fs  &log_str = (

	progressStart ("Exporting scene..." )

	--Primero exportamos grafo de navegación de la escena
	local navFillename = "nav_"+subfileName
	if(exportNavGraph navFillename) then
	(
		-- Empiezo xml
		fs = "<scene subFile='"+subfileName+"' nav='"+navFillename+"'>\n"
	)
	else
	(
		-- Empiezo xml
		fs = "<scene subFile='"+subfileName+"'>\n"
	)
	
	--Primero exportamos la info dela luz direccional
	exportDirLight &fs
	
	--Exportamos diosa si procede
	exportGoddess &fs
	
	--Primero se exportan los enemigos
	exportEnemies &fs
	
	--Exportamos luces aqui pq resulta que las omnis no son convertibles a editable poly
	exportPointLights &fs
	
	--Se exportan las meshes animadas
	exportAnimeshes &fs
	
	-- Para todos los objectos en la escena
	local i_obj = 0
	
	for obj in $* do (
		if canConvertTo obj Editable_Mesh  then (
			obj.name = toLower obj.name
			export_scene_obj obj &fs &log_str
		)
		i_obj = i_obj+1
		local percent = ( 100 * i_obj ) / $*.count 
		if not progressUpdate percent then exit
	)
	
	--Aquí exportamos cámaras
	export_cameras &fs
	
	--Se exportan los puntos de respawn para el player
	exportRespawns &fs
	
	-- Fin xml
	fs = fs + "</scene>\n"
	
	progressEnd()
)

-- Exporta la escena a un fichero en el directorio marcado por el .ini
fn export_scene_to_file filename subfile = (
	local full_path = mcv_data_path + "scenes\\" + filename
	local msg = ("exporting "+full_path + "...")
	print msg
	
	subfileName = subfile
	makeDir (mcv_data_path + "meshes\\" + subfileName + "\\")

	local temp_string
	local log_str = ""
	export_scene &temp_string &log_str
	
	-- Crear el fichero xml en format texto. Se borra si existe.
 	local myFile = openFile full_path mode:"wt"
 	if myFile == undefined then (
 		throw ( "Can't create output scene xml " + full_path )
 	)
	
	format temp_string to:myFile
 	close myFile
	
	-- Crear log. Se borra si existe.
	local log_path = mcv_data_path + "export.log"
 	local logFile = openFile log_path mode:"wt"
 	if logFile == undefined then (
 		throw ( "Can't create log file: " + full_path )
 	)
	
	format log_str to:logFile
 	close logFile
	
	format "export.log updated\n"
)

--export_scene listener
--export_scene_to_file "sonidos" "sonidos"
