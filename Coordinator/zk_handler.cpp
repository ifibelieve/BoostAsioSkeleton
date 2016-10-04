#include "zk_handler.h"

using namespace std;

ZkHandler::ZkHandler() {
	zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);
	myid_.client_id = 0;
	Init();
}

ZkHandler::~ZkHandler() {
	Close();
}

void ZkHandler::Watcher(zhandle_t *zh, int type, int state, const char *path, void *context) {
	ZkHandler *zkHandler = static_cast<ZkHandler *>(context);	
	zkHandler->ZkWatcher(zh, type, state, path, context);
}

void ZkHandler::ZkWatcher(zhandle_t *zh, int type, int state, const char *path, void *context) {
	fprintf(stderr, "\ton watcher\n");
        if (type == ZOO_SESSION_EVENT) {
                if (state == ZOO_CONNECTED_STATE) {
                        const clientid_t *id = zoo_client_id(zh_);
                        if (myid_.client_id == 0 || myid_.client_id != id->client_id) {
                                myid_ = *id;
                        }
                }
        }
	else if (type == ZOO_CHILD_EVENT) {
		RefreshMembers();
        }
        fprintf(stderr, "\tend watcher\n");
}

void ZkHandler::CreateGroup(string name) {
	group_ = "/" + name;

        Stat stat;
        if (ZNONODE == zoo_exists(zh_, group_.c_str(), 1, &stat)) {
                zoo_create(zh_, group_.c_str(), NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
        }
}

void ZkHandler::CreateMember(std::string name, std::string data) {
	string path = group_ + "/" + name;

        int rc = zoo_create(zh_, path.c_str(), data.c_str(), data.size(), &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0);
        if (ZOK != rc) {
                fprintf(stderr, "\trc: %d\n", rc);
        }
}

std::vector<std::string> ZkHandler::GetMembers() {
	if (0 == members_.size()) {
		RefreshMembers();
	}
	
	return members_;
}

void ZkHandler::Close() {
	if (nullptr != zh_) {
		zookeeper_close(zh_);
	}
}

void ZkHandler::RefreshMembers() {
	String_vector children;
        zoo_get_children(zh_, group_.c_str(), 1, &children);

	for (int i = 0; i < children.count; i++) {
		members_.push_back(children.data[i]);
		cerr << "\t[ZkHandler::RefreshMembers] " << children.data[i] << endl;
	}
}

void ZkHandler::Init() {
	if (0 == myid_.client_id) {
		zh_ = zookeeper_init("localhost:2181", ZkHandler::Watcher, 5000, 0, this, 0);
	} else {
		zh_ = zookeeper_init("localhost:2181", ZkHandler::Watcher, 5000, &myid_, this, 0);
	}
}
