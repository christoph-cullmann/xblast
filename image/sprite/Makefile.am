install-game-data:
	$(INSTALL) -d $(DESTDIR)${game_datadir}/image/sprite
	for f in *.pbm; do \
		$(install_sh_DATA) $${f} $(DESTDIR)${game_datadir}/image/sprite/$${f}; \
	done
	for f in *.ppm; do \
		$(install_sh_DATA) $${f} $(DESTDIR)${game_datadir}/image/sprite/$${f}; \
	done
	for f in *.epm; do \
		$(install_sh_DATA) $${f} $(DESTDIR)${game_datadir}/image/sprite/$${f}; \
	done

