#include <iostream>
#include <vector>
#include <zookeeper/zookeeper.h>

class ZkHandler 
{
public:
	ZkHandler();
	~ZkHandler();	

	static void Watcher(zhandle_t *zh, int type, int state, const char *path, void *context);
	void ZkWatcher(zhandle_t *zh, int type, int state, const char *path, void *context);

	void CreateGroup(std::string name);
	void CreateMember(std::string name, std::string data);
	std::vector<std::string> GetMembers();
	void Close();
private:
	void Init();
	void RefreshMembers();	

	zhandle_t *zh_;
	clientid_t myid_;
	std::string group_;
	std::vector<std::string> members_;
};
