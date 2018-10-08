install-game-data:
	$(INSTALL) -d $(DESTDIR)${game_datadir}/level
	for f in *.xal; do \
		$(install_sh_DATA) $${f} $(DESTDIR)${game_datadir}/level/$${f}; \
	done

