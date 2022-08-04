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

#include "HelloWorldScene.h"

USING_NS_CC;

Size visibleSize;
Vec2 origin;

int bgmID;
int seID;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Scene::init())
    {
        return false;
    }

    visibleSize = Director::getInstance()->getVisibleSize();
    origin = Director::getInstance()->getVisibleOrigin();

    bgmID = AudioEngine::INVALID_AUDIO_ID;
    seID = AudioEngine::INVALID_AUDIO_ID;

    auto bg = Sprite::create("background.png");
    bg->setPosition(Vec2(visibleSize * 0.5f) + origin);
    bg->setScale(visibleSize.width / bg->getContentSize().width);
    bg->getTexture()->setAliasTexParameters();
    addChild(bg);

    Vector<SpriteFrame*> aniFrames;
    aniFrames.pushBack(SpriteFrame::create("river.png", Rect(0, 0, 160, 32)));
    aniFrames.pushBack(SpriteFrame::create("river.png", Rect(0, 32, 160, 32)));

    auto anim = Animation::createWithSpriteFrames(aniFrames, 0.5f);
    auto riverSpr = Sprite::create("river.png", Rect(0, 0, 160, 32));
    riverSpr->getTexture()->setAliasTexParameters();
    riverSpr->setPosition(80, 28);
    bg->addChild(riverSpr);
    riverSpr->runAction(RepeatForever::create(
        Animate::create(anim)
    ));

    SpriteFrameCache::getInstance()->addSpriteFrame(SpriteFrame::create("little_witch_sheet_58x46_per_frame.png", Rect(0, 0, 58, 46)), "WITCH0");
    SpriteFrameCache::getInstance()->addSpriteFrame(SpriteFrame::create("little_witch_sheet_58x46_per_frame.png", Rect(58, 0, 58, 46)), "WITCH1");

    auto scoreLabel = Label::createWithBMFont("pressstart_.txt", "SCORE 00");
    scoreLabel->setPosition(Vec2(visibleSize.width * 0.5f - 10, visibleSize.height - 20) + origin);
    scoreLabel->setBMFontSize(12);
    scoreLabel->setName("SCORE");
    scoreLabel->setAnchorPoint(Vec2(1, 0));
    scoreLabel->setColor(Color3B(255, 240, 20));
    addChild(scoreLabel, 1);

    auto loopLabel = Label::createWithBMFont("pressstart_.txt", "LOOP 00");
    loopLabel->setBMFontSize(12);
    loopLabel->setName("LOOP");
    loopLabel->setPosition(Vec2(visibleSize.width * 0.5f + 10, visibleSize.height - 20) + origin);
    loopLabel->setAnchorPoint(Vec2(0, 0));
    loopLabel->setColor(Color3B(160, 240, 255));
    addChild(loopLabel, 1);

    SetTitle();

    return true;
}
void HelloWorld::SetTitle()
{
    auto title = TitleLayer::create();
    title->SetCB([=](void) {
        GameStart();
        });
    addChild(title);
}
void HelloWorld::GameStart()
{
    score = 0;
    loops = 0;
    downSpeed = 0.0f;

    ((Label*)getChildByName("SCORE"))->setString("SCORE 00");

    auto witch = Sprite::createWithSpriteFrameName("WITCH0");
    witch->getTexture()->setAliasTexParameters();
    witch->setPosition(0, 96);
    witch->setScale(0.75f);
    witch->setAnchorPoint(Vec2(0.6f, 0.3f));
    addChild(witch);

    Vector<SpriteFrame*> witchAnim;
    witchAnim.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("WITCH0"));
    witchAnim.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("WITCH1"));

    witch->runAction(RepeatForever::create(
        Animate::create(Animation::createWithSpriteFrames(witchAnim, 0.25f))
    ));
    witch->runAction(Sequence::create(
        MoveTo::create(2.0f, Vec2(80, 96)),
        CallFunc::create([=](void) {
            if (bgmID != AudioEngine::INVALID_AUDIO_ID)
                AudioEngine::stop(bgmID);

            bgmID = AudioEngine::play2d("sound/bgm_maoudamashii_8bit13.mp3", 0.5f);

            WitchRun(witch);
            }),
        NULL
    ));

    auto reddot = Sprite::create("reddot.png");
    reddot->setPosition(witch->getPosition());
    reddot->setName("REDDOT");
    reddot->getTexture()->setAliasTexParameters();
    addChild(reddot, 1);

    reddot->schedule([=](float dt) {
        reddot->setPosition(witch->getPosition());
        }, "REDDOT");

    reddot->setVisible(DEBUGMODE);

    CreateMap(1);
}
void HelloWorld::WitchRun(Sprite* witch)
{
    TMXTiledMap* map = (TMXTiledMap*)getChildByName("MAP");
    Sprite* reddot = (Sprite*)getChildByName("REDDOT");

    witch->schedule([=](float dt) {

        witch->setPosition(witch->getPosition() + Vec2(0, downSpeed));
        if (witch->getPosition().y < 0)
            downSpeed = 0.0f;
        else
            downSpeed -= 0.075f;

        auto obsLayer = map->getLayer("obs");
        auto itemLayer = map->getLayer("item");
        auto pos = witch->getPosition();
        auto tileSize = map->getTileSize();

        int tileX = (int)((pos.x - map->getPosition().x) / tileSize.width);
        int tileY = (int)((map->getContentSize().height - pos.y + map->getPosition().y) / tileSize.height);

        if ((0 <= tileX && tileX < map->getMapSize().width) && (0 <= tileY && tileY < map->getMapSize().height))
        {
            auto gid = obsLayer->getTileGIDAt(Vec2(tileX, tileY));
            if (gid > 13)
            {
                AudioEngine::stop(bgmID);

                witch->stopAllActions();
                witch->unschedule("WITCH");
                map->unschedule("MAP");
                map->stopAllActions();
                reddot->unschedule("REDDOT");

                witch->runAction(Sequence::create(
                    DelayTime::create(1.0f),
                    EaseOut::create(MoveBy::create(0.75f, Vec2(0, -visibleSize.height)), 0.5f),
                    CallFunc::create([=](void) {

                        auto gameover = GameoverLayer::create();
                        gameover->SetCB([=](void) {
                            if (map)
                                map->removeFromParent();
                            if (witch)
                                witch->removeFromParent();
                            if (reddot)
                                reddot->removeFromParent();

                            SetTitle();
                            });
                        addChild(gameover, 1);
                        }),
                    NULL
                ));

                _eventDispatcher->removeAllEventListeners();

            }

            if (ITEM_MODE == 1)
            {
                Vector<Node*> deletes;
                for (const auto element : map->getChildren())
                {
                    if (element->getName() == "HEART")
                    {
                        auto bbox = element->getBoundingBox();
                        bbox.origin = element->getParent()->convertToWorldSpace(bbox.origin);
                        if (origin.x <= bbox.origin.x && bbox.origin.x < visibleSize.width + origin.x)
                        {
                            if (bbox.containsPoint(pos))
                            {
                                deletes.pushBack(element);
                                score++;
                                ((Label*)getChildByName("SCORE"))->setString(StringUtils::format("SCORE %02d", score));

                                if (seID == AudioEngine::INVALID_AUDIO_ID)
                                {
                                    seID = AudioEngine::play2d("sound/se_coinget_1.mp3", false, 1.0f);
                                    AudioEngine::setFinishCallback(seID, [=](int id, std::string key) {
                                        seID = AudioEngine::INVALID_AUDIO_ID;
                                        });
                                }
                            }
                        }
                    }
                }
                for (const auto del : deletes)
                {
                    del->removeFromParent();
                }
            }
            else
            {
                auto gid2 = itemLayer->getTileGIDAt(Vec2(tileX, tileY));
                if (gid2 == 2)
                {
                    log("get heart");
                    itemLayer->setTileGID(1, Vec2(tileX, tileY));
                    score++;
                    ((Label*)getChildByName("SCORE"))->setString(StringUtils::format("SCORE %02d", score));

                    if (seID == AudioEngine::INVALID_AUDIO_ID)
                    {
                        seID = AudioEngine::play2d("sound/se_coinget_1.mp3", false, 1.0f);
                        AudioEngine::setFinishCallback(seID, [=](int id, std::string key) {
                            seID = AudioEngine::INVALID_AUDIO_ID;
                            });
                    }
                }
            }
        }

        }, "WITCH");

    auto touchEvent = EventListenerTouchOneByOne::create();
    touchEvent->onTouchBegan = [=](Touch* touch, Event* event) {
        return true;
    };
    touchEvent->onTouchEnded = [=](Touch* touch, Event* event) {
        downSpeed = 2.0f;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchEvent, this);
}
void HelloWorld::CreateMap(int num)
{
    auto map = TMXTiledMap::create(StringUtils::format("stage_%02d.tmx", num));
    map->setName("MAP");
    map->getLayer("grid")->setVisible(DEBUGMODE);
    map->setPosition(origin + Vec2(visibleSize.width, 0));
    addChild(map);
    map->runAction(RepeatForever::create(
        MoveBy::create(0.3f, Vec2(-16, 0))
    ));
    map->schedule([=](float dt) {
        if (map->getPositionX() < (-1) * (map->getMapSize().width * map->getTileSize().width))
        {
            map->removeFromParent();
            CreateMap(num);
        }
        }, "MAP");

    if (ITEM_MODE == 1)
    {
        map->getLayer("item")->setVisible(false);
        auto objLayer = map->getObjectGroup("obj");
        for (const auto element : objLayer->getObjects())
        {
            ValueMap ele = element.asValueMap();
            float itemX = ele["x"].asFloat();
            float itemY = ele["y"].asFloat();
            auto heart = Sprite::create("bigheart.png");
            heart->setPosition(itemX, itemY);
            heart->setName("HEART");
            map->addChild(heart);
        }
    }

    auto loopLogo = Label::createWithBMFont("pressstart_.txt", "");
    loopLogo->setBMFontSize(16);
    loopLogo->setPosition(Vec2(visibleSize * 0.5f) + origin);
    loopLogo->setColor(Color3B::WHITE);
    addChild(loopLogo, 2);
    loopLogo->runAction(Sequence::create(
        Blink::create(2.0f, 6),
        CallFunc::create([=](void) {
            if (bgmID != AudioEngine::INVALID_AUDIO_ID)
                AudioEngine::stop(bgmID);

            bgmID = AudioEngine::play2d("sound/bgm_maoudamashii_8bit13.mp3", 0.5f);
            }),
        RemoveSelf::create(),
                NULL
                ));

    score += (10 * loops);
    loops++;
    ((Label*)getChildByName("LOOP"))->setString(StringUtils::format("LOOP %02d", loops));
    ((Label*)getChildByName("SCORE"))->setString(StringUtils::format("SCORE %02d", score));

    if (loops == 1)
        loopLogo->setString("START !!");
    else
        loopLogo->setString(StringUtils::format("LOOP %d", loops));
}

TitleLayer* TitleLayer::create()
{
    TitleLayer* layer = new TitleLayer();

    if (layer)
    {
        if (layer->init())
        {
            return layer;
        }
    }
    CC_SAFE_DELETE(layer);
    return NULL;
}

bool TitleLayer::init()
{
    if (!Layer::init())
        return false;

    AudioEngine::stop(bgmID);

    auto logo = Sprite::create("titlelogo_.png");
    logo->getTexture()->setAliasTexParameters();
    logo->setPosition(origin + Vec2(visibleSize * 0.5f) + Vec2(0, visibleSize.height + logo->getContentSize().height * 0.5f));
    addChild(logo);

    logo->runAction(Sequence::create(
        EaseOut::create(MoveBy::create(0.4f, Vec2(0, -1 * (visibleSize.height + logo->getContentSize().height * 0.5f))), 0.5f),
        EaseIn::create(MoveBy::create(0.3f, Vec2(0, 50)), 0.5f),
        EaseOut::create(MoveBy::create(0.3f, Vec2(0, -50)), 0.5f),
        EaseIn::create(MoveBy::create(0.3f, Vec2(0, 20)), 0.5f),
        EaseOut::create(MoveBy::create(0.3f, Vec2(0, -20)), 0.5f),
        CallFunc::create([=](void) {

            auto touchStart = Label::createWithBMFont("pressstart_.txt", "TOUCH To START");
            touchStart->setBMFontSize(10);
            touchStart->setPosition(logo->getPosition() + Vec2(0, -60));
            touchStart->setColor(Color3B::WHITE);
            addChild(touchStart);

            auto touchEvent = EventListenerTouchOneByOne::create();
            touchEvent->onTouchBegan = [=](Touch* touch, Event* event) {
                return true;
            };
            touchEvent->onTouchEnded = [=](Touch* touch, Event* event) {
                AudioEngine::stop(bgmID);
                cb();
                this->removeFromParent();
            };
            _eventDispatcher->addEventListenerWithSceneGraphPriority(touchEvent, this);

            auto keyEvent = EventListenerKeyboard::create();
            keyEvent->onKeyReleased = [=](EventKeyboard::KeyCode keyCode, Event* event) {
                if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE)
                {
                    AudioEngine::end();
                    Director::getInstance()->end();
                }
            };
            _eventDispatcher->addEventListenerWithSceneGraphPriority(keyEvent, this);
            }),
        NULL
    ));

    if (bgmID != AudioEngine::INVALID_AUDIO_ID)
        AudioEngine::stop(bgmID);
    bgmID = AudioEngine::play2d("sound/bgm_maoudamashii_8bit22.mp3", 0.5f);

    return true;
}
void TitleLayer::SetCB(FUNC _func)
{
    cb = _func;
}

GameoverLayer* GameoverLayer::create()
{
    GameoverLayer* layer = new GameoverLayer();

    if (layer)
    {
        if (layer->init())
        {
            return layer;
        }
    }
    CC_SAFE_DELETE(layer);
    return NULL;
}

bool GameoverLayer::init()
{
    if (!Layer::init())
        return false;

    AudioEngine::stop(bgmID);

    auto logo = Sprite::create("gameoverlogo.png");
    logo->setPosition(origin + Vec2(visibleSize * 0.5f));
    logo->getTexture()->setAliasTexParameters();
    logo->setOpacity(0);
    addChild(logo);

    logo->runAction(Sequence::create(
        FadeIn::create(1.0f),
        CallFunc::create([=](void) {

            AudioEngine::play2d("se_maoudamashii_se_sound20.mp3", false, 1.0f);

            auto touchEvent = EventListenerTouchOneByOne::create();
            touchEvent->onTouchBegan = [=](Touch* touch, Event* event) {
                return true;
            };
            touchEvent->onTouchEnded = [=](Touch* touch, Event* event) {
                //Director::getInstance()->replaceScene(HelloWorld::create());
                cb();
                this->removeFromParent();
            };
            _eventDispatcher->addEventListenerWithSceneGraphPriority(touchEvent, this);
            }),
        NULL
    ));

    return true;
}
void GameoverLayer::SetCB(FUNC _func)
{
    cb = _func;
}