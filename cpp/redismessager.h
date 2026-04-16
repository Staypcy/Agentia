#include<QObject>
#include<hiredis.h>

class redisMessager:public QObject{
    Q_OBJECT
public:
    redisMessager(QObject*parent);
    ~redisMessager();


signals:

private:
    redisContext*context;

};
