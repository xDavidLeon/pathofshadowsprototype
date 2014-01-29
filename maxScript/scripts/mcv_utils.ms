
-- Devuelve un array, donde el elemento i-esimo contiene el array de las 
-- face id's que usan el material i-esimo (tienen mat_id = i )
function get_materials_used_by_obj obj = (
	
	local nfaces = getNumFaces obj
	local unique_mats = #()
	for face_id = 1 to nfaces do (
		-- Que material id  usa esta cara?
		local mat_id = getFaceMatID obj face_id
		
		-- Si la entrada i-esima no es un array, lo creo
		if unique_mats[ mat_id ] == undefined then (
			unique_mats[ mat_id ] = #()
		)
		-- Añado la cara al grupo del material
		append unique_mats[ mat_id ] face_id
	)
	
	return unique_mats
)

function matrix_as_string m = (
	local ss = stringstream ""
	format "% % % " m.row1.x m.row1.y m.row1.z to:ss
	format "% % % " m.row2.x m.row2.y m.row2.z to:ss
	format "% % % " m.row3.x m.row3.y m.row3.z to:ss
	format "% % %"  m.row4.x m.row4.y m.row4.z to:ss
	return ss as string                                  
)

--Exporta grafo de navegación
function exportNavGraph filename =
(
	--Pillar layer navigation
	local nav_layer = LayerManager.getLayerFromName "navigation"
	if nav_layer == undefined then
	(
		format "WARNING: There's no 'navigation' layer in the file.\n"
		return false
	)
	
	--Metemos el contenido de la layer en un vector
	local nodeVector = #()
	nav_layer.nodes &nodeVector
	if nodeVector == #() then throw ("There's nothing into 'navigation' layer!!!\n")
	
	--Exportar grafo
	local n_nodes = nodeVector.count
	local str_stream_nodes = "" as stringStream
	local str_stream_edges = "" as stringStream
	
	format "\t<nodelist>\n" to:str_stream_nodes
	format "\t<edgelist>\n" to:str_stream_edges
	
	for node in nodeVector do
	(
		local node_id = replace node.name 1 3 "" as integer --substituimos las posiciones 1 a 3 por nada, es decir, pasamos de "navNNN" a "NNN"
		local pos = node.position
		
		--Es el nodo del fin de nivel?
		if getUserProp node "final" != undefined then
		(
			format "\t\t<node id=\"%\" pos_x=\"%\" pos_y=\"%\" pos_z=\"%\" final=\"true\" />\n" node_id pos.x pos.z -pos.y to: str_stream_nodes
		)
		else 
		(
			format "\t\t<node id=\"%\" pos_x=\"%\" pos_y=\"%\" pos_z=\"%\" />\n" node_id pos.x pos.z -pos.y to: str_stream_nodes
		)
		
		format "\t\t<edges id=\"%\">\n" node_id to:str_stream_edges
		
		local neighbours_str = getUserProp node "neighbours" as string
		local neighbours = filterString neighbours_str " "
		local n_neighbours = neighbours.count
		for neighbour in neighbours do
		(
			local neighbour_id = neighbour as integer
			local dist = distance nodeVector[n_nodes-node_id+1] nodeVector[n_nodes-neighbour_id+1]
			
			format "\t\t\t<edge id=\"%\" distance=\"%\"/>\n" neighbour dist to:str_stream_edges
		)
		format "\t\t</edges>\n" node_id to:str_stream_edges
	)
	
	format "\t</nodelist>\n" to:str_stream_nodes
	format "\t</edgelist>\n" to:str_stream_edges
	
	local xml_string = ""
	xml_string+="<graph>\n"
	xml_string+=str_stream_nodes
	xml_string+=str_stream_edges
	xml_string+="</graph>\n"
	--format xml_string
	
	-- Crear el fichero xml. Se borra si existe.
	local full_path = mcv_data_path + "scenes\\" + filename
 	local myFile = openFile full_path mode:"wt"
 	if myFile == undefined then (
 		throw ( "Can't create output scene xml " + full_path )
 	)
	format xml_string to:myFile
	
 	close myFile
	format "Exported: %\n" filename
	return true
)

fn exportEnemyInfo enemy &fs =
(
	local first_point = getKnotPoint enemy 1 1
	local tempStream = "" as stringstream
	
	format "		<name>%</name>\n" enemy.name  to:tempStream
	format "		<type>ENEMY</type>\n" to:tempStream
	format "		<transform>1 0 0 0 1 0 0 0 1 % % %</transform>\n" first_point.x first_point.z -first_point.y to:tempStream
	format "		<model>\n" to:tempStream
	
	local characterName = getUserProp enemy "character"
	if (characterName == "xu") then
	(
		format "			<character>xu</character>\n" to:tempStream
		format "		<visible>true</visible>\n" to:tempStream
		format "			<mat>\n" to:tempStream
		format "				<material_name>tech_skin</material_name>\n"  to:tempStream
		format "				<diffuse>hardcoded/xu_d</diffuse>\n"  to:tempStream
		format "				<normalmap>hardcoded/xu_n</normalmap>\n" to:tempStream
		format "			</mat>\n" to:tempStream
	)
	else if  characterName == "xu_shield" then
	(
		format "			<character>xu_shield</character>\n" to:tempStream
		format "		<visible>true</visible>\n" to:tempStream
		format "			<mat>\n" to:tempStream
		format "				<material_name>tech_skin</material_name>\n"  to:tempStream
		format "				<diffuse>hardcoded/xu_escudo_d</diffuse>\n"  to:tempStream
		format "				<normalmap>hardcoded/xu_escudo_n</normalmap>\n" to:tempStream
		format "			</mat>\n" to:tempStream
	)
	else if  characterName == "xu_farolillo" then
	(
		format "			<character>xu_farolillo</character>\n" to:tempStream
		format "		<visible>true</visible>\n" to:tempStream
		format "			<mat>\n" to:tempStream
		format "				<material_name>tech_skin</material_name>\n"  to:tempStream
		format "				<diffuse>hardcoded/xu_farolillo_d</diffuse>\n"  to:tempStream
		format "				<emissive>hardcoded/xu_farolillo_i</emissive>\n"  to:tempStream
		format "				<normalmap>hardcoded/xu_farolillo_n</normalmap>\n" to:tempStream
		format "			</mat>\n" to:tempStream
	)
	else if  characterName == "one" then
	(
		format "			<character>one</character>\n" to:tempStream
		format "		<visible>true</visible>\n" to:tempStream
		format "			<mat>\n" to:tempStream
		format "				<material_name>tech_skin</material_name>\n"  to:tempStream
		format "				<diffuse>hardcoded/one_d</diffuse>\n"  to:tempStream
		format "				<normalmap>hardcoded/one_n</normalmap>\n" to:tempStream
		format "			</mat>\n" to:tempStream
	)
	else
	(
		format "TROLOLOLOLO, LOLOLO, LOLOLOOOOOO\n"
	)
	
	format "			<aabb>-0.581745 -0.00498399 0.507839 0.409982 1.52444 -0.800522</aabb>\n" to:tempStream
	format "		</model>\n" to:tempStream
	format "		<physics shape='CHARACTER' static='false' kinematic='false'/>\n" to:tempStream
	
	local disabled = getUserProp enemy "disabled"
	if(disabled != undefined) then format "		<disabled />\n" to:tempStream
	fs = fs+tempStream
)

--Exporta un patrullero
function exportPatrolInfo patrol &fs=
(
	local n_knots = numknots patrol 
	if n_knots < 2 then throw ( "el patrullero % tiene menos de 2 puntos!!\n" patrol.name)
	local tempStream = "" as stringstream
	
	format "\t\t<ai type='patroller' />\n"  to:tempStream
	format "\t\t<patrol_points>\n" to:tempStream
	
	local i_knot = 1
	
	while i_knot <= n_knots do
	(
		local knot = getKnotPoint patrol 1 i_knot
		
		format "\t\t\t<waypoint pos_x=\"%\" pos_y=\"%\" pos_z=\"%\" />\n" knot.x knot.z -knot.y to:tempStream
		i_knot = i_knot+1
	)
	
	format "\t\t</patrol_points>\n" to:tempStream
	
	fs = fs+tempStream
	
	format "Exported patroler: %\n" patrol.name
)

--Exporta un gatekeeper
function exportGatekeeperlInfo gk &fs=
(
	local tempStream = "" as stringstream
	local n_knots = numknots gk
	if n_knots != 2 then throw ( "the gatekeep line must have 2 points!!")
	local str_stream = "" as stringStream
	
	format "\t\t<ai type='gatekeeper' />\n" to:tempStream
	format "\t\t<gatekeep_points>\n" to:tempStream
	
	local knot = getKnotPoint gk 1 1
	format "\t\t\t<gk_pos pos_x=\"%\" pos_y=\"%\" pos_z=\"%\" />\n" knot.x knot.z -knot.y to: tempStream
	
	knot = getKnotPoint gk 1 2
	format "\t\t\t<lookAt pos_x=\"%\" pos_y=\"%\" pos_z=\"%\" />\n" knot.x knot.z -knot.y to: tempStream
	
	format "\t\t</gatekeep_points>\n" to:tempStream
	
	local talkTo = getUserProp gk "talk_to"
	if talkTo != undefined then
	(
		if talkTo != "" then
		(
			format "\t\t<talk_to enemy=\"%\"/>\n" talkTo to:tempStream
		)
	)
	
	fs = fs+tempStream
	
	format "Exported gatekeeper: %\n" gk.name
)

--Exporta los enemigos en la escena, de cada tipo
fn exportEnemies &fs  =
(
	--patrolers
	local pat_layer = LayerManager.getLayerFromName "patrolers"
	if pat_layer == undefined then format "No se han detectado patrulleros\n"
	else
	(
		local patrolers_v = #()
		pat_layer.nodes &patrolers_v
		if patrolers_v == #() then( format "Capa patrolers vacia->No se exportan patrulleros\n")
		else
		( 
			for patroler in patrolers_v do
			(
				fs = fs + "	<obj>\n" 
				--Info genérica
				exportEnemyInfo patroler &fs
				--Info específica
				exportPatrolInfo patroler &fs
				
				fs = fs + "	</obj>\n"
			) 
		)
	)
		
	--gatekeepers
	local gk_layer = LayerManager.getLayerFromName "gatekeepers"
	if gk_layer == undefined then format "No se han detectado gatekeepers\n"
	else
	(
		local gatekeepers_v = #()
		gk_layer.nodes &gatekeepers_v
		if gatekeepers_v == #() then( format "Capa gatekeepers vacia->No se exportan gatekeepers\n")
		else
		( 
			for gatekeeper in gatekeepers_v do
			(
				fs = fs + "	<obj>\n" 
				--Info genérica
				exportEnemyInfo gatekeeper &fs
				--Info específica
				exportGatekeeperlInfo gatekeeper &fs
				
				fs = fs + "	</obj>\n"
			) 
		)
	)
)

--Exporta la info de 1 cámara
fn export_camera cam_id positions lookAts &fs =
(
	if(numknots positions < 4) then
	(
		format "WARNING: pos of cam % has least than 4 points!! (ignoring camera)" cam_id
		return false
	)
	
	if(numknots lookAts < 4) then
	(
		format "WARNING: lookAts of cam % has least than 4 points!! (ignoring camera)" cam_id
		return false
	)
	
	local tempStream = "" as stringstream
	
	--datos de tiempo de la cámara
	local tr_time = getUserProp positions "time"
	if tr_time == undefined then (tr_time = 3.0)
	local start_delay = getUserProp positions "start_delay"
	if start_delay == undefined then (start_delay = 1.0)
	local finish_delay = getUserProp positions "finish_delay"
	if finish_delay == undefined then (finish_delay = 2.0)
	--datos de interpolacion inicial/final
	local from_player = getUserProp positions "from_player"
	if from_player == undefined then (from_player = false)
	else from_player = true
	local to_player = getUserProp positions "to_player"
	if to_player == undefined then (to_player = false)
	else to_player = true
	
	format "\t\t<cam_timing id=\"%\" time=\"%\" start_delay=\"%\" finish_delay=\"%\" " cam_id tr_time start_delay finish_delay to:tempStream
	
	if from_player==true then (format "from_player=\"true\" " to:tempStream)
	if to_player==true then (format "to_player=\"true\" " to:tempStream)
	
	format "/>\n" to:tempStream
	
	--posiciones
	local n_knots = numknots positions 
	local i_knot = 1
	
	while i_knot <= n_knots do
	(
		local knot = getKnotPoint positions 1 i_knot
		
		format "\t\t<cam_pos id=\"%\" x=\"%\" y=\"%\" z=\"%\" />\n" cam_id knot.x knot.z -knot.y to:tempStream
		i_knot = i_knot+1
	)
	
	--lookAts
	n_knots = numknots lookAts 
	i_knot = 1
	
	while i_knot <= n_knots do
	(
		local knot = getKnotPoint lookAts 1 i_knot
		
		format "\t\t<cam_lookAt id=\"%\" x=\"%\" y=\"%\" z=\"%\" />\n" cam_id knot.x knot.z -knot.y to:tempStream
		i_knot = i_knot+1
	)
	
	fs = fs + tempStream
)

--Exporta la info de las "cámaras" de la escena
fn export_cameras &fs = 
(
	--comprobamos capa
	local cam_layer = LayerManager.getLayerFromName "cameras"
	if cam_layer == undefined then format "No se ha detectado layer con camaras\n"
	else
	(
		local things_v = #()
		cam_layer.nodes &things_v
		if things_v == #() then( format "Capa cameras vacia->No se exportan camaras\n")
		else
		( 
			if (things_v.count == 1) then
			(
				format "WARNING: la capa cameras debería tener al menos 2 splines!\n"
				return false
			)
			
			fs = fs + "\t<cameras_info>\n" 
			for thing in things_v do
			(
				--para cada pos...
				if (thing.name[1] == "p") then
				(
					--obtener id
					local id = (filterString thing.name "_")[2]
					--buscar el look con la misma id (solo para asegurar que se exportan las 2 cosas)
					local hasLookAt = false
					local i=0
					while(i<=things_v.count-1 and hasLookAt==false) do
					(
						i = i+1
						if (things_v[i].name == "look_"+(id as string)) then hasLookAt = true
					)
					if(hasLookAt == false) then
					(
						format ("WARNING: pos_"+(id as string)+" no tiene un look\n")
						continue
					)
					--Exportar pos y look de esa cámara
					export_camera id thing things_v[i] &fs
				)
			) 
			fs = fs + "\t</cameras_info>\n"
		)
	)
)

fn exportDirLight &fs =
(
	local moon_layer = LayerManager.getLayerFromName "moon"
	if moon_layer == undefined then 
	(
		fs = fs + "\t<moon x='-1' y='-1' z='-1' />\n"
	)
	
		if moon_layer == undefined then return
	
	local lights_v = #()
	moon_layer.nodes &lights_v
	if (lights_v == #() or lights_v.count == 1) then( throw "Layer lights: Hace falta una luz direccional y un target dentro suyo\n")
	
	local moon
	for thing in lights_v do ( if(thing.name == "moon") then moon = thing	)
	
	local light_dir = normalize (moon.Target.position - moon.position)
	
	fs = fs + "\t<moon x='"+(light_dir.x as string)+"' y='"+(light_dir.z as string)+"' z='"+((-light_dir.y) as string)+"' />\n"
)

function export_light_obj obj &fs = (
	
	local tempStream = ""  as stringstream
	
	format "	<obj>\n" to:tempStream
	format "		<name>%</name>\n" obj.name to:tempStream
	format "		<type>LIGHT</type>\n" to:tempStream
	
	-- Convertir la matrix de transformacion al espacio del mcv
	local mtx_max = obj.transform
	local max2mcv = (quat 90 [1,0,0] ) as matrix3
	local mcv2max = inverse max2mcv
	local mtx_mcv = mcv2max * mtx_max * max2mcv
	local mtx_str = matrix_as_string mtx_mcv
	
	format "		<transform>%</transform>\n" mtx_str to:tempStream
	
	format "		<light>\n" to:tempStream
	local light_type = getUserProp obj "light_type"
	if (light_type != undefined) then (
		format "			<light_type>%</light_type>\n" light_type to:tempStream
	)
	else (
		format "			<light_type>POINT</light_type>\n" to:tempStream
	)
	local light_color = obj.rgb
	if (light_color != undefined) then (
		format "			<light_color>%</light_color>\n" light_color to:tempStream
	)
	local light_intensity = getUserProp obj "light_intensity"
	if (light_intensity != undefined) then (
		format "			<light_intensity>%</light_intensity>\n" light_intensity to:tempStream
	)
	local light_direction = getUserProp obj "light_direction"
	if (light_direction != undefined) then (
		format "			<light_direction>%</light_direction>\n" light_direction to:tempStream
	)
	--local radius = length (obj.max - obj.min)/2  --Yo, Alvaro, comento esto a sabiendas que lenght no hace el calculo bien
	local radius = obj.farAttenStart  --para omnilights
	--local radius = obj.radius --para esferas
	format "			<light_radius>%</light_radius>\n" radius to:tempStream
	
	format "		</light>\n" to: tempStream
	format "	</obj>\n" to:tempStream
	
	fs = fs+tempStream
)

fn exportPointLights &fs =
(
	--comprobamos capa
	local pl_layer = LayerManager.getLayerFromName "pointlights"
	if pl_layer == undefined then format "No se ha detectado layer con pointlights\n"
	else
	(
		local pls_v = #()
		pl_layer.nodes &pls_v
		if pls_v == #() then( format "Capa pointlights vacia->No se exportan pointlights\n")
		else
		( 
			for pl in pls_v do
			(
				export_light_obj pl &fs
			) 
		)
	)
)

fn export_animesh animesh type &fs = 
(
	local tempStream = "" as stringstream
	
	format "	<obj>\n"  to:tempStream
	format "		<name>%</name>\n" animesh.name  to:tempStream
	format "		<type>ANIMATED</type>\n" to:tempStream
	
	-- Convertir la matrix de transformacion al espacio del mcv
	local mtx_max = animesh.transform
	local max2mcv = (quat 90 [1,0,0] ) as matrix3
	local mcv2max = inverse max2mcv
	local mtx_mcv = mcv2max * mtx_max * max2mcv
	local mtx_str = matrix_as_string mtx_mcv
	
	format "		<transform>%</transform>\n" mtx_str to:tempStream
	
	
	format "		<model>\n" to:tempStream
	format "			<visible>true</visible>\n" to:tempStream
	format "			<character>%</character>\n" type to:tempStream
	format "			<mat>\n" to:tempStream
	format "				<material_name>tech_skin</material_name>\n"  to:tempStream
	
	if(type=="animesh_molino") then
	(
		format "\t\t\t\t<diffuse>hardcoded/molino_d</diffuse>\n"  to:tempStream
	)
	else if(type=="animesh_banderas_1") then
	(
		format "\t\t\t\t<diffuse>hardcoded/flags_animesh</diffuse>\n"  to:tempStream
	)
	else if(type=="animesh_farolillos_1") then
	(
		format "\t\t\t\t<diffuse>hardcoded/farol_d_animesh1</diffuse>\n"  to:tempStream
		format "\t\t\t\t<emissive>hardcoded/farol_d_animesh1</emissive>\n"  to:tempStream
	)
	else if(type=="animesh_farolillos_2") then
	(
		format "\t\t\t\t<diffuse>hardcoded/farol_d_animesh2</diffuse>\n"  to:tempStream
		format "\t\t\t\t<emissive>hardcoded/farol_d_animesh2</emissive>\n"  to:tempStream
	)
	else if(type=="animesh_door") then
	(
		format "\t\t\t\t<diffuse>hardcoded/door</diffuse>\n"  to:tempStream
	)
	else
	(
		format "\t\t\t\t<!-- No se que meter aqui... -->"
	)
	
	format "			</mat>\n" to:tempStream
	format "			<aabb>-0.581745 -0.00498399 0.507839 0.409982 1.52444 -0.800522</aabb>\n" to:tempStream
	format "		</model>\n" to:tempStream
	format "	</obj>\n"  to:tempStream
	
	fs = fs+tempStream
)

fn export_animeshes_in_layer layer_name &fs =
(
	--comprobamos capa
	local layer = LayerManager.getLayerFromName layer_name
	if layer == undefined then return false
	else
	(
		local v = #()
		layer.nodes &v
		if v == #() then return false
		else
		( 
			for thing in v do
			(
				export_animesh thing layer_name &fs
			) 
		)
	)
)

fn exportAnimeshes &fs =
(	
	export_animeshes_in_layer "animesh_molino" &fs
	export_animeshes_in_layer "animesh_banderas_1" &fs
	export_animeshes_in_layer "animesh_farolillos_1" &fs
	export_animeshes_in_layer "animesh_farolillos_2" &fs
	export_animeshes_in_layer "animesh_door" &fs
)

fn export_respawn rp &fs =
(
	local tempStream = "" as stringstream
	
	local mtx_max = rp.transform
	local max2mcv = (quat 90 [1,0,0] ) as matrix3
	local mcv2max = inverse max2mcv
	local mtx_mcv = mcv2max * mtx_max * max2mcv
	local mtx_str = matrix_as_string mtx_mcv
	
	format "\t\t<respawn id='%' transform='%' />\n" rp.name mtx_str to: tempStream
	
	fs = fs+tempStream
)

--Exporta los puntos (y orientaciones) de revivir del player
fn exportRespawns &fs =
(
	--comprobamos capa
	local rp_layer = LayerManager.getLayerFromName "respawns"
	if rp_layer == undefined then format "No se ha detectado layer con respawns\n"
	else
	(
		local rps_v = #()
		rp_layer.nodes &rps_v
		if rps_v == #() then( format "Capa respawns vacia->No se exportan respawns\n")
		else
		( 
			fs = fs+ "\t<respawns>\n"
			for rp in rps_v do
			(
				export_respawn rp &fs
			) 
			fs = fs+ "\t</respawns>\n"
		)
	)
)

fn exportGoddess &fs =
(
	local g_layer = LayerManager.getLayerFromName "goddess"
	if g_layer == undefined then (return false)
	
	local tempStream = "" as stringstream
	
	local v = #()
	g_layer.nodes &v
	
	
	format	"	<obj>\n" to:tempStream
	-- obj name
		format	"		<name>goddess_crow</name>\n" to:tempStream
	-- obj type
		format	"		<type>GODDESS</type>\n" obj_type to:tempStream
	
	-- Convertir la matrix de transformacion al espacio del mcv
	local mtx_max = v[1].transform
	local max2mcv = (quat 90 [1,0,0] ) as matrix3
	local mcv2max = inverse max2mcv
	local mtx_mcv = mcv2max * mtx_max * max2mcv
	local mtx_str = matrix_as_string mtx_mcv
	
	-- transform
		format	"		<transform>%</transform>\n" mtx_str to:tempStream
	-- export model component with mesh+materials
		format 	"		<model>\n"  to:tempStream
		format	"			<character>raven</character>\n"  to:tempStream
		format	"			<mat>\n"  to:tempStream
		format	"				<material_name>tech_crow</material_name>\n"  to:tempStream
		format	"				<diffuse>hardcoded/raven_i</diffuse>\n"  to:tempStream
		format	"				<emissive>hardcoded/raven_d</emissive>\n"  to:tempStream
		format	"			</mat>\n"  to:tempStream
		format 	"			<visible>true</visible>\n" to:tempStream
		format	"			<aabb>-0.581745 -0.00498399 0.507839 0.409982 1.52444 -0.800522</aabb>\n"  to:tempStream
		format	"		</model>\n" to:tempStream
		
	format	"	</obj>\n" to:tempStream
	
	format "Goddess exported\n"
	
	fs = fs+tempStream;
)









--exportNavgraph()
--exportPatrol $ "patrol.xml"
--exportGateKeep $ "gatekeep.xml"