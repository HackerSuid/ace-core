#ifndef SENSORY_CODEC_FACTORY_H_
#define SENSORY_CODEC_FACTORY_H_

#include <map>

template <class T>
class SensoryCodecFactory
{
public:
    SensoryCodecFactory() {}
    ~SensoryCodecFactory() {}

    T* Get()
    {
        return new T;
    }
};

template <class T, class KeyType>
class SensoryCodecGeneralFactory
{
public:
    ~SensoryCodecGeneralFactory()
    {
        typename std::map<KeyType, SensoryCodecFactory<T> *>::iterator it =
            CodecCtorMap.begin();
        while (it != CodecCtorMap.end())
        {
            delete (*it).second;
            ++it;
        }
    }

    void Register(KeyType id, SensoryCodecFactory<T> *Ctor)
    {
        CodecCtorMap[id] = Ctor;
    }

    T* Get(KeyType id)
    {
        return CodecCtorMap[id]->Get();
    }

private:
    std::map<KeyType, SensoryCodecFactory<T> *> CodecCtorMap;
};

#endif

