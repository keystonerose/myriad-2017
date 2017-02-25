#include "engine.hpp"

#include <QApplication>
#include <QThread>

int main(int argc, char** argv) {
    
    QApplication app{argc, argv};

    myriad::engine worker;
    QThread worker_thread;
    worker.moveToThread(&worker_thread);
    worker_thread.start();

    worker_thread.quit();
    worker_thread.wait();

    return 0;
}
