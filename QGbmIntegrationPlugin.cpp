#include <qpa/qplatformintegrationplugin.h>
#include "QGbmIntegration.h"

namespace
{
    class Plugin final : public QPlatformIntegrationPlugin
    {
        Q_OBJECT
        Q_PLUGIN_METADATA( IID QPlatformIntegrationFactoryInterface_iid FILE "metadata.json" )

      public:

        QPlatformIntegration* create( const QString& system, const QStringList& ) override
        {
            if ( system.compare( QStringLiteral( "gbm" ), Qt::CaseInsensitive ) == 0 )
            {
                return new QGbmIntegration();
            }

            return nullptr;
        }
    };
}

#include "QGbmIntegrationPlugin.moc"

