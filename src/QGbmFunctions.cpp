#include "QGbmFunctions.h"

#include <private/qguiapplication_p.h>

/*
    definitions included from qguiapplication_p.h do conflict
    with includes from other libs. So we better hide QGuiApplicationPrivate 
 */
QInputDeviceManager* QGbm::inputDeviceManager()
{
    return QGuiApplicationPrivate::inputDeviceManager();
}

