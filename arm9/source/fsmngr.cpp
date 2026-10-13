#include "fsmngr.h"
#include "fifotool.h"

cFSManager::cFSManager() : _isSDInserted(false), _isFlashcart(false), _fsRoot() {

}

void cFSManager::init() {
    // We're always on a flashcart if fat: is accessible
    if (access("fat:/", F_OK) == 0) {
        _isFlashcart = true;
        _fsRoot = "fat:";
    } else {
        _isFlashcart = false;
        _fsRoot = "sd:";
    }

    if (access("sd:/", F_OK) == 0){
        _isSDInserted = true;
    } else {
        _isSDInserted = false;
    }
}

bool cFSManager::isFlashcart() const {
    return _isFlashcart;
}

bool cFSManager::isSDInserted() const {
    return _isSDInserted;
}


std::string cFSManager::resolveSystemPath(const char* path) const {
    return _fsRoot + path;
}

std::string cFSManager::getFSRoot() const {
    return _fsRoot + "/";
}

/*
bool cFSManager::checkSDInserted() const {
    fifoSendValue32(FIFO_USER_01, MENU_MSG_IS_SD_INSERTED);

    fifoWaitValue32(FIFO_USER_01);

    int result = fifoGetValue32(FIFO_USER_01);

    return result != 0;
}
*/
