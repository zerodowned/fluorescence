
set (NET_HPP
    net/manager.hpp
    net/encryption.hpp
    net/xorencryption.hpp
    net/twofishencryption.hpp
    net/noencryption.hpp
    net/decompress.hpp
    net/socket.hpp
    net/packet.hpp
    net/packetlist.hpp
    net/packetwriter.hpp
    net/packetreader.hpp
    net/multipurposepacket.hpp
    net/walkpacketmanager.hpp
    )

set (NET_CPP
    net/manager.cpp
    net/xorencryption.cpp
    net/twofishencryption.cpp
    net/noencryption.cpp
    net/decompress.cpp
    net/socket.cpp
    net/packet.cpp
    net/packetwriter.cpp
    net/packetreader.cpp
    net/multipurposepacket.cpp
    net/walkpacketmanager.cpp
    )

set (PACKETS_HPP
    net/packets/unknown.hpp
    net/packets/basetext.hpp

    net/packets/00_createcharacter.hpp
    net/packets/02_movementrequest.hpp
    net/packets/05_attackrequest.hpp
    net/packets/06_doubleclick.hpp
    net/packets/07_pickupitem.hpp
    net/packets/08_dropitem.hpp
    net/packets/09_singleclick.hpp
    net/packets/11_statupdatefull.hpp
    net/packets/12_useskill.hpp
    net/packets/13_equiprequest.hpp
    net/packets/1a_worlditem.hpp
    net/packets/1b_playerinit.hpp
    net/packets/1c_asciitext.hpp
    net/packets/1d_deleteobject.hpp
    net/packets/20_teleport.hpp
    net/packets/21_movementdeny.hpp
    net/packets/22_movementaccept.hpp
    net/packets/24_displaycontainer.hpp
    net/packets/25_containerupdate.hpp
    net/packets/27_pickupreject.hpp
    net/packets/2e_equippeditem.hpp
    net/packets/34_statskillquery.hpp
    net/packets/3a_skilllockchange.hpp
    net/packets/3a_skillsupdate.hpp
    net/packets/3c_containercontent.hpp
    net/packets/54_playsound.hpp
    net/packets/55_logincomplete.hpp
    net/packets/5d_characterselect.hpp
    net/packets/65_weather.hpp
    net/packets/6c_target.hpp
    net/packets/6d_playmusic.hpp
    net/packets/6e_mobileanimation.hpp
    net/packets/72_warmode.hpp
    net/packets/73_ping.hpp
    net/packets/77_nakedmobile.hpp
    net/packets/78_equippedmobile.hpp
    net/packets/7c_objectpicker.hpp
    net/packets/7d_objectpickerresponse.hpp
    net/packets/80_loginrequest.hpp
    net/packets/82_loginreject.hpp
    net/packets/83_characterdelete.hpp
    net/packets/85_characterdeletereject.hpp
    net/packets/86_characterlistupdate.hpp
    net/packets/88_openpaperdoll.hpp
    net/packets/8c_serverredirect.hpp
    net/packets/91_gameserverloginrequest.hpp
    net/packets/9b_helprequest.hpp
    net/packets/9e_vendorselllist.hpp
    net/packets/9f_vendorselloffer.hpp
    net/packets/a0_gameserverselect.hpp
    net/packets/a1_statupdatehitpoints.hpp
    net/packets/a2_statupdatemana.hpp
    net/packets/a3_statupdatestamina.hpp
    net/packets/a8_serverlist.hpp
    net/packets/a9_characterlist.hpp
    net/packets/ad_speechrequest.hpp
    net/packets/ae_unicodetext.hpp
    net/packets/b0_gumpmenu.hpp
    net/packets/b1_gumpreply.hpp
    net/packets/b8_profile.hpp
    net/packets/bd_clientversion.hpp
    net/packets/c0_osieffect.hpp
    net/packets/c1_localizedtext.hpp
    net/packets/c2_unicodeprompt.hpp
    net/packets/c2_unicodepromptreply.hpp
    net/packets/c7_osieffectextended.hpp
    net/packets/c8_updaterange.hpp
    net/packets/cc_localizedtextaffix.hpp
    net/packets/d6_objectproperties.hpp
    net/packets/d6_objectpropertyrequest.hpp
    net/packets/dc_objectpropertieshash.hpp
    net/packets/dd_compressedgumpmenu.hpp
    net/packets/df_buff.hpp
    net/packets/e2_mobileanimation_sa.hpp
    net/packets/ef_seed.hpp
    net/packets/f3_worldobject.hpp

    net/packets/bf.hpp
    net/packets/bf/01_fastwalkinit.hpp
    net/packets/bf/02_fastwalkupdate.hpp
    net/packets/bf/04_closegump.hpp
    net/packets/bf/08_changemap.hpp
    net/packets/bf/13_contextmenurequest.hpp
    net/packets/bf/14_opencontextmenu.hpp
    net/packets/bf/15_contextmenureply.hpp
    net/packets/bf/1b_spellbookcontent.hpp
    net/packets/bf/1c_castspell.hpp
    )

set (PACKETS_CPP
    net/packets/unknown.cpp
    net/packets/basetext.cpp

    net/packets/00_createcharacter.cpp
    net/packets/02_movementrequest.cpp
    net/packets/05_attackrequest.cpp
    net/packets/06_doubleclick.cpp
    net/packets/07_pickupitem.cpp
    net/packets/08_dropitem.cpp
    net/packets/09_singleclick.cpp
    net/packets/11_statupdatefull.cpp
    net/packets/12_useskill.cpp
    net/packets/13_equiprequest.cpp
    net/packets/1a_worlditem.cpp
    net/packets/1b_playerinit.cpp
    net/packets/1c_asciitext.cpp
    net/packets/1d_deleteobject.cpp
    net/packets/20_teleport.cpp
    net/packets/21_movementdeny.cpp
    net/packets/22_movementaccept.cpp
    net/packets/24_displaycontainer.cpp
    net/packets/25_containerupdate.cpp
    net/packets/27_pickupreject.cpp
    net/packets/2e_equippeditem.cpp
    net/packets/34_statskillquery.cpp
    net/packets/3a_skilllockchange.cpp
    net/packets/3a_skillsupdate.cpp
    net/packets/3c_containercontent.cpp
    net/packets/54_playsound.cpp
    net/packets/55_logincomplete.cpp
    net/packets/5d_characterselect.cpp
    net/packets/65_weather.cpp
    net/packets/6c_target.cpp
    net/packets/6d_playmusic.cpp
    net/packets/6e_mobileanimation.cpp
    net/packets/72_warmode.cpp
    net/packets/73_ping.cpp
    net/packets/77_nakedmobile.cpp
    net/packets/78_equippedmobile.cpp
    net/packets/7c_objectpicker.cpp
    net/packets/7d_objectpickerresponse.cpp
    net/packets/80_loginrequest.cpp
    net/packets/82_loginreject.cpp
    net/packets/83_characterdelete.cpp
    net/packets/85_characterdeletereject.cpp
    net/packets/86_characterlistupdate.cpp
    net/packets/88_openpaperdoll.cpp
    net/packets/8c_serverredirect.cpp
    net/packets/91_gameserverloginrequest.cpp
    net/packets/9b_helprequest.cpp
    net/packets/9e_vendorselllist.cpp
    net/packets/9f_vendorselloffer.cpp
    net/packets/a0_gameserverselect.cpp
    net/packets/a1_statupdatehitpoints.cpp
    net/packets/a2_statupdatemana.cpp
    net/packets/a3_statupdatestamina.cpp
    net/packets/a8_serverlist.cpp
    net/packets/a9_characterlist.cpp
    net/packets/ad_speechrequest.cpp
    net/packets/ae_unicodetext.cpp
    net/packets/b0_gumpmenu.cpp
    net/packets/b1_gumpreply.cpp
    net/packets/b8_profile.cpp
    net/packets/bd_clientversion.cpp
    net/packets/c0_osieffect.cpp
    net/packets/c1_localizedtext.cpp
    net/packets/c2_unicodeprompt.cpp
    net/packets/c2_unicodepromptreply.cpp
    net/packets/c7_osieffectextended.cpp
    net/packets/c8_updaterange.cpp
    net/packets/cc_localizedtextaffix.cpp
    net/packets/d6_objectproperties.cpp
    net/packets/d6_objectpropertyrequest.cpp
    net/packets/dc_objectpropertieshash.cpp
    net/packets/dd_compressedgumpmenu.cpp
    net/packets/df_buff.cpp
    net/packets/e2_mobileanimation_sa.cpp
    net/packets/ef_seed.cpp
    net/packets/f3_worldobject.cpp

    net/packets/bf.cpp
    net/packets/bf/01_fastwalkinit.cpp
    net/packets/bf/02_fastwalkupdate.cpp
    net/packets/bf/04_closegump.cpp
    net/packets/bf/08_changemap.cpp
    net/packets/bf/13_contextmenurequest.cpp
    net/packets/bf/14_opencontextmenu.cpp
    net/packets/bf/15_contextmenureply.cpp
    net/packets/bf/1b_spellbookcontent.cpp
    net/packets/bf/1c_castspell.cpp
    )

set (MD5
    net/md5/global.h
    net/md5/md5.h
    net/md5/md5c.c
    )

set (TWOFISH
    net/twofish/aes.h
    net/twofish/debug.h
    net/twofish/platform.h
    net/twofish/table.h
    net/twofish/twofish.c
    )

set(NET_FILES ${NET_HPP} ${NET_CPP} ${MD5} ${TWOFISH} ${PACKETS_HPP} ${PACKETS_CPP} PARENT_SCOPE)
