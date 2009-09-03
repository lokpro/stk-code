//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "config/player.hpp"
#include "config/user_config.hpp"
#include "kart_selection.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "states_screens/state_manager.hpp"
#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "input/device_manager.hpp"
#include "input/input_device.hpp"
#include "items/item_manager.hpp"
#include "io/file_manager.hpp"
#include "karts/kart.hpp"
#include "karts/kart_properties_manager.hpp"
#include "utils/translation.hpp"
#include "utils/random_generator.hpp"
#include "utils/string_utils.hpp"

#include <string>

InputDevice* player_1_device = NULL;

using namespace GUIEngine;

namespace KartSelectionScreen
{
    class PlayerKartWidget;
    
    // ref only since we're adding them to a Screen, and the Screen will take ownership of these widgets
    // FIXME : delete these objects when leaving the screen (especially when using escape)
    ptr_vector<PlayerKartWidget, REF> g_player_karts;
    
#if 0
#pragma mark -
#pragma mark PlayerKartWidget
#endif
    
    class PlayerKartWidget : public Widget
    {
        float x_speed, y_speed, w_speed, h_speed;

    public:
        LabelWidget* playerIDLabel;
        SpinnerWidget* playerName;
        ModelViewWidget* modelView;
        LabelWidget* kartName;
        
        ActivePlayer* m_associatedPlayer;

        int playerID;
        std::string spinnerID;
        
        int player_id_x, player_id_y, player_id_w, player_id_h;
        int player_name_x, player_name_y, player_name_w, player_name_h;
        int model_x, model_y, model_w, model_h;
        int kart_name_x, kart_name_y, kart_name_w, kart_name_h;
        int m_irrlicht_widget_ID;
        
        int target_x, target_y, target_w, target_h;

        LabelWidget *getPlayerIDLabel() {return playerIDLabel;}
        std::string deviceName;
        std::string m_kartInternalName;
        
        PlayerKartWidget(ActivePlayer* associatedPlayer, Widget* area, const int playerID, const int irrlichtWidgetID=-1) : Widget()
        {
            m_associatedPlayer = associatedPlayer;
            x_speed = 1.0f;
            y_speed = 1.0f;
            w_speed = 1.0f;
            h_speed = 1.0f;

            m_irrlicht_widget_ID = irrlichtWidgetID;

            this->playerID = playerID;
            this->m_properties[PROP_ID] = StringUtils::insertValues("@p%i", playerID);
            
            setSize(area->x, area->y, area->w, area->h);
            target_x = x;
            target_y = y;
            target_w = w;
            target_h = h;
            
            if (associatedPlayer->getDevice()->getType() == DT_KEYBOARD)
            {
                deviceName += "keyboard";
            }
            else if (associatedPlayer->getDevice()->getType() == DT_GAMEPAD)
            {
                deviceName += "gamepad";
            }
            
            playerIDLabel = new LabelWidget();
            
            playerIDLabel->m_text = 
                //I18N: In kart selection screen (Will read like 'Player 1 (foobartech gamepad)')
                StringUtils::insertValues(_("Player %i (%s)"), playerID + 1, deviceName.c_str()); 
            
            playerIDLabel->m_properties[PROP_TEXT_ALIGN] = "center";
            playerIDLabel->m_properties[PROP_ID] = StringUtils::insertValues("@p%i_label", playerID);
            playerIDLabel->x = player_id_x;
            playerIDLabel->y = player_id_y;
            playerIDLabel->w = player_id_w;
            playerIDLabel->h = player_id_h;
            //playerID->setParent(this);
            m_children.push_back(playerIDLabel);
            
            playerName = new SpinnerWidget();
            playerName->x = player_name_x;
            playerName->y = player_name_y;
            playerName->w = player_name_w;
            playerName->h = player_name_h;
            
            spinnerID = StringUtils::insertValues("@p%i_spinner", playerID);
            
            const int playerAmount = UserConfigParams::m_all_players.size();
            playerName->m_properties[PROP_MIN_VALUE] = "0";
            playerName->m_properties[PROP_MAX_VALUE] = (playerAmount-1);
            playerName->m_properties[PROP_ID] = spinnerID;
            //playerName->setParent(this);
            m_children.push_back(playerName);
            
            
            playerName->m_event_handler = this;
            
            modelView = new ModelViewWidget();
            
            modelView->x = model_x;
            modelView->y = model_y;
            modelView->w = model_w;
            modelView->h = model_h;
            modelView->m_properties[PROP_ID] = StringUtils::insertValues("@p%i_model", playerID);
            //modelView->setParent(this);
            m_children.push_back(modelView);
            
            // Init kart model
            std::string& default_kart = UserConfigParams::m_default_kart;
            const KartProperties* props = kart_properties_manager->getKart(default_kart);
            KartModel* kartModel = props->getKartModel();
            
            this->modelView->addModel( kartModel->getModel() );
            this->modelView->addModel( kartModel->getWheelModel(0), kartModel->getWheelGraphicsPosition(0) );
            this->modelView->addModel( kartModel->getWheelModel(1), kartModel->getWheelGraphicsPosition(1) );
            this->modelView->addModel( kartModel->getWheelModel(2), kartModel->getWheelGraphicsPosition(2) );
            this->modelView->addModel( kartModel->getWheelModel(3), kartModel->getWheelGraphicsPosition(3) );
            
            kartName = new LabelWidget();
            kartName->m_text = props->getName();
            kartName->m_properties[PROP_TEXT_ALIGN] = "center";
            kartName->m_properties[PROP_ID] = StringUtils::insertValues("@p%i_kartname", playerID);
            kartName->x = kart_name_x;
            kartName->y = kart_name_y;
            kartName->w = kart_name_w;
            kartName->h = kart_name_h;
            //kartName->setParent(this);
            m_children.push_back(kartName);
        }
        
        ~PlayerKartWidget()
        {
            if (playerIDLabel->getIrrlichtElement() != NULL)
                playerIDLabel->getIrrlichtElement()->remove();
            
            if (playerName->getIrrlichtElement() != NULL)
                playerName->getIrrlichtElement()->remove();
            
            if (modelView->getIrrlichtElement() != NULL)
                modelView->getIrrlichtElement()->remove();
            
            if (kartName->getIrrlichtElement() != NULL)
                kartName->getIrrlichtElement()->remove();
            
            getCurrentScreen()->manualRemoveWidget(this);
        }
        
        void setPlayerID(const int newPlayerID)
        {
            if (StateManager::get()->getActivePlayers().get(newPlayerID) != m_associatedPlayer)
            {
                printf("Player: %p\nIndex: %d\nm_associatedPlayer: %p\n", StateManager::get()->getActivePlayers().get(newPlayerID), newPlayerID, m_associatedPlayer);
                std::cerr << "Internal inconsistency, PlayerKartWidget has IDs and pointers that do not correspond to one player\n";
                assert(false);
            }
            
            playerID = newPlayerID;
            
            //I18N: In kart selection screen (Will read like 'Player 1 (foobartech gamepad)')
            irr::core::stringw  newLabel = StringUtils::insertValues(_("Player %i (%s)"), playerID + 1, deviceName.c_str());
            playerIDLabel->setText( newLabel ); 
            playerIDLabel->m_properties[PROP_ID] = StringUtils::insertValues("@p%i_label", playerID);
        }
        
        virtual void add()
        {
            playerIDLabel->add();
            
            // the first player will have an ID of its own to allow for keyboard navigation despite this widget being added last
            if (m_irrlicht_widget_ID != -1) playerName->m_reserved_id = m_irrlicht_widget_ID;
            playerName->add();
            
            modelView->add();
            kartName->add();
            
            modelView->update(0);
            
            // TODO : only fill list on first add
            const int playerAmount = UserConfigParams::m_all_players.size();
            for(int n=0; n<playerAmount; n++)
            {
                playerName->addLabel( UserConfigParams::m_all_players[n].getName() );
            }
            
        }
                
        void move(const int x, const int y, const int w, const int h)
        {
            target_x = x;
            target_y = y;
            target_w = w;
            target_h = h;
            
            x_speed = abs( this->x - x ) / 300.0f;
            y_speed = abs( this->y - y ) / 300.0f;
            w_speed = abs( this->w - w ) / 300.0f;
            h_speed = abs( this->h - h ) / 300.0f;
        }
        
        void onUpdate(float delta)
        {
            if (target_x == x && target_y == y && target_w == w && target_h == h) return;
            
            int move_step = (int)(delta*1000.0f);
            
            // move x towards target
            if (x < target_x)
            {
                x += (int)(move_step*x_speed);
                if (x > target_x) x = target_x; // don't move to the other side of the target
            }
            else if (x > target_x)
            {
                x -= (int)(move_step*x_speed);
                if (x < target_x) x = target_x; // don't move to the other side of the target
            }
            
            // move y towards target
            if (y < target_y)
            {
                y += (int)(move_step*y_speed);
                if (y > target_y) y = target_y; // don't move to the other side of the target
            }
            else if (y > target_y)
            {
                y -= (int)(move_step*y_speed);
                if (y < target_y) y = target_y; // don't move to the other side of the target
            }
            
            // move w towards target
            if (w < target_w)
            {
                w += (int)(move_step*w_speed);
                if (w > target_w) w = target_w; // don't move to the other side of the target
            }
            else if (w > target_w)
            {
                w -= (int)(move_step*w_speed);
                if (w < target_w) w = target_w; // don't move to the other side of the target
            }
            // move h towards target
            if (h < target_h)
            {
                h += (int)(move_step*h_speed);
                if (h > target_h) h = target_h; // don't move to the other side of the target
            }
            else if (h > target_h)
            {
                h -= (int)(move_step*h_speed);
                if (h < target_h) h = target_h; // don't move to the other side of the target
            }
            
            setSize(x, y, w, h);
            
            playerIDLabel->move(player_id_x,
                                player_id_y,
                                player_id_w,
                                player_id_h);
            playerName->move(player_name_x,
                             player_name_y,
                             player_name_w,
                             player_name_h );
            modelView->move(model_x,
                            model_y,
                            model_w,
                            model_h);
            kartName->move(kart_name_x,
                           kart_name_y,
                           kart_name_w,
                           kart_name_h);
            
        }
// disabled but some events were disptached twice (e.g. spinner clicks)
#if 0
        virtual bool transmitEvent(Widget* w, std::string& originator, const int playerID)
        {
            std::cout << "= kart selection :: transmitEvent " << originator << "=\n";
            Widget* topmost = w;
            /* Find topmost parent. Stop looping if a widget event handler's is itself, to not fall
             in an infinite loop (this can happen e.g. in checkboxes, where they need to be
             notified of clicks onto themselves so they can toggle their state. )
             */
            while (topmost->m_event_handler != NULL && topmost->m_event_handler != topmost)
            {
                // transmit events to all listeners in the chain
                std::cout << "transmitting event to widget " << topmost->m_type << std::endl;
                if (!topmost->transmitEvent(w, originator, playerID)) return false;
                topmost = topmost->m_event_handler;
                
                std::string name = topmost->m_properties[PROP_ID];
                
                if (name == spinnerID)
                {
                    m_associatedPlayer->setPlayerProfile( UserConfigParams::m_all_players.get(playerName->getValue()) );
                    
                    // transmit events to all listeners in the chain
                    std::cout << "transmitting event to widget " << topmost->m_type << std::endl;
                    if (!topmost->transmitEvent(w, originator, playerID)) return false;
                    
                    return false; // do not continue propagating the event
                }

            }
         
            return true; // continue propagating the event
        }
#endif
        void setSize(const int x, const int y, const int w, const int h)
        {
            this->x = x;
            this->y = y;
            this->w = w;
            this->h = h;
            
            // -- sizes
            player_id_w = w;
            player_id_h = 25;
            
            player_name_h = 40;
            player_name_w = std::min(400, w);
            
            kart_name_w = w;
            kart_name_h = 25;
            
            // for shrinking effect
            if (h < 175)
            {
                const float factor = h / 175.0f;
                kart_name_h   = (int)(kart_name_h*factor);
                player_name_h = (int)(player_name_h*factor);
                player_id_h   = (int)(player_id_h*factor);
            }
            
            // --- layout
            player_id_x = x;
            player_id_y = y;
            
            player_name_x = x + w/2 - player_name_w/2;
            player_name_y = y + player_id_h;

            const int modelMaxHeight =  h - kart_name_h - player_name_h - player_id_h;
            const int modelMaxWidth =  w;
            const int bestSize = std::min(modelMaxWidth, modelMaxHeight);
            const int modelY = y + player_name_h + player_id_h;
            model_x = x + w/2 - (int)(bestSize/2);
            model_y = modelY + modelMaxHeight/2 - bestSize/2;
            model_w = (int)(bestSize);
            model_h = bestSize;
            
            kart_name_x = x;
            kart_name_y = y + h - kart_name_h;
        }
        void setKartInternalName(const std::string& whichKart)
        {
            m_kartInternalName = whichKart;
        }
    };
    
#if 0
#pragma mark -
#pragma mark KartHoverListener
#endif
    
class KartHoverListener : public DynamicRibbonHoverListener
    {
    public:
        void onSelectionChanged(DynamicRibbonWidget* theWidget, const std::string& selectionID,
                                const irr::core::stringw& selectionText, const int playerID)
        {
            ModelViewWidget* w3 = g_player_karts[playerID].modelView;
            assert( w3 != NULL );
            
            if (selectionID == "gui/track_random.png")
            {
                scene::IMesh* model = item_manager->getItemModel(Item::ITEM_BONUS_BOX);
                w3->clearModels();
                w3->addModel( model, Vec3(0.0f, 0.0f, -12.0f) );
                w3->update(0);
                g_player_karts[playerID].kartName->setText( _("Random Kart") );
            }
            else
            {
                //printf("%s\n", selectionID.c_str());
                const KartProperties* kart = kart_properties_manager->getKart(selectionID);
                if (kart == NULL) return;
                KartModel* kartModel = kart->getKartModel();
                
                w3->clearModels();
                w3->addModel( kartModel->getModel() );
                w3->addModel( kartModel->getWheelModel(0), kartModel->getWheelGraphicsPosition(0) );
                w3->addModel( kartModel->getWheelModel(1), kartModel->getWheelGraphicsPosition(1) );
                w3->addModel( kartModel->getWheelModel(2), kartModel->getWheelGraphicsPosition(2) );
                w3->addModel( kartModel->getWheelModel(3), kartModel->getWheelGraphicsPosition(3) );
                w3->update(0);

                g_player_karts[playerID].kartName->setText( selectionText.c_str() );
            }

            g_player_karts[playerID].setKartInternalName(selectionID);
        }
    };
KartHoverListener* karthoverListener = NULL;

#if 0
#pragma mark -
#pragma mark Functions
#endif
    
// Return true if event was handled successfully
bool playerJoin(InputDevice* device, bool firstPlayer)
{
    std::cout << "playerJoin() ==========\n";

    DynamicRibbonWidget* w = getCurrentScreen()->getWidget<DynamicRibbonWidget>("karts");
    if (w == NULL)
    {
        std::cerr << "playerJoin(): Called outside of kart selection screen.\n";
        return false;
    }
    else if (device == NULL)
    {
        std::cerr << "playerJoin(): Passed null pointer\n";
        return false;
    }

    // ---- Get available area for karts
    // make a copy of the area, ands move it to be outside the screen
    Widget rightarea = *getCurrentScreen()->getWidget("playerskarts");
    rightarea.x = irr_driver->getFrameSize().Width; // start at the rightmost of the screen
    
    // ---- Create new active player
    int id = StateManager::get()->createActivePlayer( UserConfigParams::m_all_players.get(0), device );
    ActivePlayer *aplayer = StateManager::get()->getActivePlayer(id);
    
    // ---- Create player/kart widget
    // FIXME : player ID needs to be synced with active player list
    PlayerKartWidget* newPlayer;
    if (firstPlayer)
        newPlayer = new PlayerKartWidget(aplayer, &rightarea, g_player_karts.size(), rightarea.m_reserved_id);
    else
        newPlayer = new PlayerKartWidget(aplayer, &rightarea, g_player_karts.size());
    
    getCurrentScreen()->manualAddWidget(newPlayer);
    newPlayer->add();

    g_player_karts.push_back(newPlayer);
    
    // ---- Divide screen space among all karts
    const int amount = g_player_karts.size();
    Widget* fullarea = getCurrentScreen()->getWidget("playerskarts");
    const int splitWidth = fullarea->w / amount;
    
    for (int n=0; n<amount; n++)
    {
        g_player_karts[n].move( fullarea->x + splitWidth*n, fullarea->y, splitWidth, fullarea->h );
    }
    
    // ---- Focus a kart for this player
    const int playerID = amount-1;
    w->setSelection(playerID, playerID);
    
    return true;
}

PlayerKartWidget* removedWidget = NULL;

// -----------------------------------------------------------------------------
// Return true if event was handled succesfully
bool playerQuit(ActivePlayer* player)
{    
    int playerID = -1;
    
    DynamicRibbonWidget* w = getCurrentScreen()->getWidget<DynamicRibbonWidget>("karts");
    if (w == NULL )
    {
        std::cout << "playerQuit() called outside of kart selection screen.\n";
        return false;
    }

    // If last player quits, return to main menu
    if (g_player_karts.size() <= 1) return false;

    std::cout << "playerQuit() ==========\n";

    for (int n=0; n<g_player_karts.size(); n++)
    {
        if (g_player_karts[n].m_associatedPlayer == player)
        {
            playerID = n;
            break;
        }
    }
    if (playerID == -1)
    {
        std::cout << "void playerQuit(ActivePlayer* player) : cannot find passed player\n";
        return false;
    }
    
    // Just a cheap way to check if there is any discrepancy 
    // between g_player_karts and the active player array
    assert( g_player_karts.size() == StateManager::get()->activePlayerCount() );

    // unset selection of this player
    // FIXME: will only work if the player that quits is the last of the list
    if (GUIEngine::g_focus_for_player[playerID] != NULL)
    {
        GUIEngine::g_focus_for_player[playerID]->unsetFocusForPlayer(playerID);
    }
    GUIEngine::g_focus_for_player[playerID] = NULL;
    
    removedWidget = g_player_karts.remove(playerID);
    StateManager::get()->removeActivePlayer(playerID);
    renumberKarts();    
    Widget* fullarea = getCurrentScreen()->getWidget("playerskarts");
    removedWidget->move( removedWidget->x + removedWidget->w/2, fullarea->y + fullarea->h, 0, 0);
    return true;
}
    
// -----------------------------------------------------------------------------
    
void kartSelectionUpdate(float delta)
{
    const int amount = g_player_karts.size();
    for (int n=0; n<amount; n++)
    {
        g_player_karts[n].onUpdate(delta);
    }
    
   if (removedWidget != NULL)
   {
       removedWidget->onUpdate(delta);
       
       if (removedWidget->w == 0 || removedWidget->h == 0)
       {
           // destruct when too small (for "disappear" effects)
           GUIEngine::getCurrentScreen()->manualRemoveWidget(removedWidget);
           delete removedWidget;
           removedWidget = NULL;
       }
   }
}

// -----------------------------------------------------------------------------
/**
 * Callback handling events from the kart selection menu
 */
void menuEventKarts(Widget* widget, const std::string& name)
{
    if(name == "tearDown")
    {
        //g_player_karts.clearWithoutDeleting();
        g_player_karts.clearAndDeleteAll();
    }
    else if(name == "init")
    {
        // FIXME: Reload previous kart selection screen state
        g_player_karts.clearAndDeleteAll();
        StateManager::get()->resetActivePlayers();
        input_manager->getDeviceList()->setAssignMode(DETECT_NEW);
       
        DynamicRibbonWidget* w = getCurrentScreen()->getWidget<DynamicRibbonWidget>("karts");
        assert( w != NULL );
        
        if (karthoverListener == NULL)
        {
            karthoverListener = new KartHoverListener();
            w->registerHoverListener(karthoverListener);
        }
        
        //Widget* area = getCurrentScreen()->getWidget("playerskarts");
        
        if (!getCurrentScreen()->m_inited)
        {            
            // Build kart list
            std::vector<int> group = kart_properties_manager->getKartsInGroup("standard");
            const int kart_amount = group.size();
            
            // add Tux (or whatever default kart) first
            std::string& default_kart = UserConfigParams::m_default_kart;
            for(int n=0; n<kart_amount; n++)
            {
                const KartProperties* prop = kart_properties_manager->getKartById(group[n]);
                if (prop->getIdent() == default_kart)
                {
                    std::string icon_path = file_manager->getDataDir() ;
                    icon_path += "/karts/" + prop->getIdent() + "/" + prop->getIconFile();
                    w->addItem(prop->getName(), prop->getIdent().c_str(), icon_path.c_str());
                    break;
                }
            }
            
            // add others
            for(int n=0; n<kart_amount; n++)
            {
                const KartProperties* prop = kart_properties_manager->getKartById(group[n]);
                if (prop->getIdent() != default_kart)
                {
                    std::string icon_path = file_manager->getDataDir() ;
                    icon_path += "/karts/" + prop->getIdent() + "/" + prop->getIconFile();
                    w->addItem(prop->getName(), prop->getIdent().c_str(), icon_path.c_str());
                }
            }
            
            getCurrentScreen()->m_inited = true;
        }
        
        /*

            TODO: Ultimately, it'd be nice to *not* delete g_player_karts so that
            when players return to the kart selection screen, it will appear as
            it did when they left (at least when returning from the track menu).
            Rebuilding the screen is a little tricky.

        */

        if (g_player_karts.size() > 0)
        {
            // FIXME: trying to rebuild the screen
            for (int n = 0; n < g_player_karts.size(); n++)
            {
                PlayerKartWidget *pkw;
                pkw = g_player_karts.get(n);
                getCurrentScreen()->manualAddWidget(pkw);
                pkw->add();
            }

        }
        else // For now this is what will happen
        {
            playerJoin( input_manager->getDeviceList()->getLatestUsedDevice(), true );
            w->updateItemDisplay();
        }
        
        // Player 0 select first kart (Tux)
        w->setSelection(0, 0);
        w->m_rows[0].requestFocus();
        
        getCurrentScreen()->m_inited = true;
    } // end if init
    
    else if (name == "kartgroups")
    {
        RibbonWidget* tabs = getCurrentScreen()->getWidget<RibbonWidget>("kartgroups");
        assert(tabs != NULL);

        std::string selection = tabs->getSelectionIDString(GUI_PLAYER_ID);

        DynamicRibbonWidget* w = getCurrentScreen()->getWidget<DynamicRibbonWidget>("karts");
        w->clearItems();
        
        // TODO : preserve selection of karts for all players
        
        if (selection == "all")
        {
            const int kart_amount = kart_properties_manager->getNumberOfKarts();
            
            for(int n=0; n<kart_amount; n++)
            {
                const KartProperties* prop = kart_properties_manager->getKartById(n);
                
                std::string icon_path = file_manager->getDataDir() ;
                icon_path += "/karts/" + prop->getIdent() + "/" + prop->getIconFile();
                w->addItem(prop->getName().c_str(), prop->getIdent().c_str(), icon_path.c_str());
            }
        }
        else
        {        
            std::vector<int> group = kart_properties_manager->getKartsInGroup(selection);
            const int kart_amount = group.size();
            
            for(int n=0; n<kart_amount; n++)
            {
                const KartProperties* prop = kart_properties_manager->getKartById(group[n]);
                
                std::string icon_path = file_manager->getDataDir() ;
                icon_path += "/karts/" + prop->getIdent() + "/" + prop->getIconFile();
                w->addItem(prop->getName().c_str(), prop->getIdent().c_str(), icon_path.c_str());
            }
        }
        
        w->updateItemDisplay();

    }
    else if (name == "karts")
    {
        DynamicRibbonWidget* w = getCurrentScreen()->getWidget<DynamicRibbonWidget>("karts");
        assert( w != NULL );
        
        ptr_vector< ActivePlayer, HOLD >& players = StateManager::get()->getActivePlayers();
        std::cout << "==========\n" << players.size() << " players :\n";
        for(int n=0; n<players.size(); n++)
        {
            std::cout << "     Player " << n << " is " << players[n].getProfile()->getName() << " on " << players[n].getDevice()->m_name << std::endl;
        }
        std::cout << "==========\n";
        
        race_manager->setNumPlayers( players.size() );
        race_manager->setNumLocalPlayers( players.size() );
        
        RandomGenerator random;
        
        //g_player_karts.clearAndDeleteAll();      
        //race_manager->setLocalKartInfo(0, w->getSelectionIDString());
        for (int n = 0; n < g_player_karts.size(); n++)
        {
            std::string selection = g_player_karts[n].m_kartInternalName;
            
            if (selection == "gui/track_random.png")
            {
                // FIXME: in multiplayer game, if two players select' random' make sure they don't select
                // the same kart or an already selected kart
                const std::vector<ItemDescription>& items = w->getItems();
                const int randomID = random.get(items.size());
                selection = items[randomID].m_code_name;
            }
            // std::cout << "selection=" << selection.c_str() << std::endl;
            
            race_manager->setLocalKartInfo(n, selection);
        }

        //Return to assign mode
        input_manager->getDeviceList()->setAssignMode(ASSIGN);

        StateManager::get()->pushMenu("racesetup.stkgui");
    }
}

// -----------------------------------------------------------------------------
    
void renumberKarts()
{
    DynamicRibbonWidget* w = getCurrentScreen()->getWidget<DynamicRibbonWidget>("karts");
    assert( w != NULL );
    Widget* fullarea = getCurrentScreen()->getWidget("playerskarts");
    const int splitWidth = fullarea->w / g_player_karts.size();

    printf("Renumbering karts...");
    for (int n=0; n < g_player_karts.size(); n++)
    {
        g_player_karts[n].setPlayerID(n);
        g_player_karts[n].move( fullarea->x + splitWidth*n, fullarea->y, splitWidth, fullarea->h );
    }

    w->updateItemDisplay();
    printf("OK\n");

}

}
