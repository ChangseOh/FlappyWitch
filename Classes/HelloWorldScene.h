/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__


#define DEBUGMODE true
#define ITEM_MODE 1

#include "cocos2d.h"
#include "audio/include/AudioEngine.h"

USING_NS_CC;

extern Size visibleSize;
extern Vec2 origin;

extern int bgmID;
extern int seID;

class HelloWorld : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);

    int score;
    int loops;
    float downSpeed;

    void SetTitle();
    void GameStart();
    void WitchRun(Sprite* witch);
    void CreateMap(int num);
};

class TitleLayer : public cocos2d::Layer
{
public:
    static TitleLayer* create();

    virtual bool init();

    typedef std::function<void()> FUNC;
    FUNC cb;
    void SetCB(FUNC _func);
};
class GameoverLayer : public cocos2d::Layer
{
public:
    static GameoverLayer* create();

    virtual bool init();

    typedef std::function<void()> FUNC;
    FUNC cb;
    void SetCB(FUNC _func);
};

#endif // __HELLOWORLD_SCENE_H__
