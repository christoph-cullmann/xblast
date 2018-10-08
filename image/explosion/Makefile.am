install-game-data:
	$(INSTALL) -d $(DESTDIR)${game_datadir}/image/explosion
	for f in *.pbm; do \
		$(install_sh_DATA) $${f} $(DESTDIR)${game_datadir}/image/explosion/$${f}; \
	done
	for f in *.ppm; do \
		$(install_sh_DATA) $${f} $(DESTDIR)${game_datadir}/image/explosion/$${f}; \
	done
	$(install_sh_DATA) expl.pos $(DESTDIR)${game_datadir}/image/explosion/expl.pos

