list=Split("""xblast.c util.c str_util.c intro.c introdat.c map.c sprite.c 
	color.c status.c player.c bomb.c action.c event.c image.c shrink.c 
	func.c info.c ini_file.c atom.c scramble.c demo.c debug.c level.c 
	random.c game_local.c game_demo.c game_server.c game_client.c game.c 
	menu.c menu_player.c menu_level.c menu_control.c menu_game.c 
	menu_network.c menu_extras.c mi_tool.c mi_base.c mi_button.c 
	mi_color.c mi_combo.c mi_cyclic.c mi_host.c mi_int.c mi_keysym.c 
	mi_label.c mi_player.c mi_string.c mi_tag.c mi_toggle.c mi_map.c 
	mi_stat.c client.c server.c central.c network.c com.c browse.c com_to_server.c 
	com_listen.c com_to_client.c com_newgame.c com_stream.c com_dg_client.c  
	com_to_central.c com_from_central.c com_central.c user.c dat_rating.c 
	com_dg_server.c com_dgram.c com_query.c com_browse.c com_reply.c 
	com_base.c net_socket.c net_tele.c net_dgram.c cfg_main.c cfg_level.c 
	cfg_player.c cfg_game.c cfg_control.c cfg_stat.c cfg_demo.c 
	cfg_xblast.c chat.c menu_edit.c x11_common.c x11_event.c x11_atom.c x11_config.c
	x11_msgbox.c	x11c_init.c x11c_image.c x11c_text.c
	x11c_tile.c x11c_sprite.c 	x11c_pixmap.c x11_sound.c
	x11_socket.c x11_joystick.c bot.c""")
opts = Options()
opts.Add(PathOption('Datadir',
                               'Path to configuration file',
                               '.'))

env = Environment(options = opts,
                             CPPDEFINES={'GAME_DATADIR' : '"$CONFIG"'},
	LIBS = Split('png m X11'),
	CPPPATH = ['/usr/include'],
        LIBPATH = ['/usr/lib','/usr/X11R6/lib'] )
conf = Configure(env)
#if not conf.CheckLib('m'):
#        print 'Did not find libm.a or m.lib, exiting!'
#        Exit(1)
#if not conf.CheckLib('X11'):
#        print 'Did not find lib X11 , exiting!'
#        Exit(1)
env = conf.Finish()
env.Program('xblast',list)