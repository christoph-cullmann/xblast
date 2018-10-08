install-game-data:
	$(INSTALL) -d $(DESTDIR)${game_datadir}/image/misc
	for f in *.pbm; do \
		$(install_sh_DATA) $${f} $(DESTDIR)${game_datadir}/image/misc/$${f}; \
	done
	for f in *.ppm; do \
		$(install_sh_DATA) $${f} $(DESTDIR)${game_datadir}/image/misc/$${f}; \
	done

