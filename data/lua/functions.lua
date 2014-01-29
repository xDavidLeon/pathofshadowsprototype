function setCamDist(dist)
	SLB.using(SLB)
	lh:setCamDist(dist);
end

function createEnemy(dist)
	SLB.using(SLB)
	currentEntity = lh:createEnemy(dist)
end

function destroyEnemy()
	SLB.using(SLB)
	lh:destroyEnemy(currentEntity)
end

sf = string.format

function loadScene(sceneName)
	SLB.using(SLB)
	lh:loadScene(sf("%s", sceneName))
end

function addCamToQueue(cam_id)
	SLB.using(SLB)
	lh:addCamToQueue(cam_id)
end

function r_graph()
	SLB.using(SLB)
	lh:r_graph()
end

--Triggers cementerio
function trigger001() --Al nacer
	SLB.using(SLB)
	lh:execCineSeq(1)
end

function trigger002() --diosa
	SLB.using(SLB)
	lh:execCineSeq(2)
end

function trigger003() --despues de la 1a verja
	SLB.using(SLB)
	lh:execCineSeq(3)
end

function trigger004() --despues de la 2a verja
	SLB.using(SLB)
	lh:execCineSeq(4)
end

function trigger006() --cima de las escaleras
	SLB.using(SLB)
	lh:disableTutorial(sf("%s", "t007"))
	lh:disableTutorial(sf("%s", "t006"))
	lh:enableTutorial(sf("%s", "t009"))
	lh:t_entity(sf("%s", "txu_p003"))
	lh:playGoddessVoice(sf("%s", "9_you are not_EDITED"))
	lh:enableSub(sf("%s", "st8"), 5)
	lh:updatePlayerRespawn(sf("%s", "r001"))
end

function trigger007() --donde se usa el señuelo
	SLB.using(SLB)
	lh:disableTutorial(sf("%s", "t009"))
	lh:enableTutorial(sf("%s", "t010"))
end

function trigger008() --cerca del ultimo xu
	SLB.using(SLB)
	lh:disableTutorial(sf("%s", "t010"))
	lh:enableTutorial(sf("%s", "t011"))
end

function trigger009() --fin de nivel
	SLB.using(SLB)
	lh:execCineSeq(5)
end

--Triggers aldea
function trigger010() --encima del puente
	SLB.using(SLB)
	--lh:execCineSeq(6)
end

function trigger011() --bifurcacion
	SLB.using(SLB)
	lh:playGoddessVoice(sf("%s", "11_sometimes2_EDITED"))
	lh:enableSub(sf("%s", "st11"), 5)
	lh:enableTutorial(sf("%s", "t012"))
	lh:updatePlayerRespawn(sf("%s", "r001"))
end

function trigger013() --resp2
	SLB.using(SLB)
	lh:updatePlayerRespawn(sf("%s", "r002"))
	lh:playGoddessVoice(sf("%s", "12_all of this_EDITED"))
	lh:enableSub(sf("%s", "st12"), 5)
end

function trigger014() --final de nivel
	SLB.using(SLB)
	lh:execCineSeq(8)
end

--Triggers patio
function trigger020() --pasillo der
	SLB.using(SLB)
	lh:updatePlayerRespawn(sf("%s", "r001"))
end

function trigger021() --pasillo izq
	SLB.using(SLB)
	lh:updatePlayerRespawn(sf("%s", "r002"))
end

function trigger023() --puente
	SLB.using(SLB)
	lh:updatePlayerRespawn(sf("%s", "r003"))
end

function trigger022() --final de nivel
	SLB.using(SLB)
	lh:execCineSeq(9)
end

function trigger024() --par de antorchas 1
	SLB.using(SLB)
end

function trigger025() --par de antorchas 2
	SLB.using(SLB)
end

function trigger026() --par de antorchas 3
	SLB.using(SLB)
end

function trigger027() --par de antorchas 4
	SLB.using(SLB)
end

--Triggers cementerio CG
function trigger028() --inicio, para que arranquen las cámaras
	SLB.using(SLB)
	lh:execCineSeq(10)
end


function updatePlayerRespawn(rp_name)
	SLB.using(SLB)
	lh:updatePlayerRespawn(sf("%s", rp_name))
end

function t_dc() --Toggle Debug Camera
	SLB.using(SLB)
	lh:tdc();
end

function t_ai() --Toggle Artificial Intelligence
	SLB.using(SLB)
	lh:t_ai()
end

function t_anim() --Toggle animations
	SLB.using(SLB)
	lh:t_anim()
end

function t_entity(entityName) --Toggle entity
	SLB.using(SLB)
	lh:t_entity(sf("%s", entityName))
end

function r_automat() --(Toggle) Render automat info
	SLB.using(SLB)
	lh:r_automat()
end

function r_ai() --(Toggle) Render AI info
	SLB.using(SLB)
	lh:r_ai()
end

function t_triggers() --Toggle triggers
	SLB.using(SLB)
	lh:t_triggers()
end

function s_ppos(rp_name) --Set player position (respawn point)
	SLB.using(SLB)
	lh:setPlayerPos(sf("%s", rp_name))
end
