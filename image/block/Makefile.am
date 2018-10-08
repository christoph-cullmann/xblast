install-game-data:
	$(INSTALL) -d $(DESTDIR)${game_datadir}/image/block
	for f in *.ppm; do \
		$(install_sh_DATA) $${f} $(DESTDIR)${game_datadir}/image/block/$${f}; \
	done

