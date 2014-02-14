#ifndef LUA_HELPER
#define LUA_HELPER

#include <string>

class LuaHelper
{
public:
	LuaHelper(void);
	~LuaHelper(void);

	static LuaHelper& get()
	{
		static LuaHelper lh;
		return lh;
	}
	
	void toggleDbgCamera();
	void setCameraDistance(float dist);

	int createEnemy(float dist);
	void destroyEnemy(int id);

	void loadScene(const std::string& sceneName);

	void toggleAI();

	void addCamToQueue(int cam_id);
	void execCineSeq(int cine_id);

	void playGoddessVoice(const std::string& rp_name);

	void disableTutorial(std::string t_name);
	void enableTutorial(std::string t_name);

	void enableSubtitle(const std::string& s_name, float time);

	void renderGraph();

	void enableEntity(const std::string& e_name);
	void disableEntity(const std::string& e_name);

	void setPlayerLife(float life);
	void setPlayerPos(const std::string& e_name);

	void updatePlayerRespawn(const std::string& rp_name);

	void toggleAnimation();
	void toggleEntity(const std::string& e_name);
	void toggleRAutomat();
	void toggleRAI();
	void toggleTriggers();

};

#endif