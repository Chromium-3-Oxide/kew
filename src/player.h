#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "appstate.h"
#include "imgfunc.h"
#include "directorytree.h"
#include "playlist.h"
#include "playlist_ui.h"
#include "search_ui.h"
#include "searchradio_ui.h"
#include "songloader.h"
#include "sound.h"
#include "term.h"
#include "utils.h"
#include "visuals.h"
#include "common_ui.h"
#include "common.h"

extern int numProgressBars;

extern bool fastForwarding;
extern bool rewinding;

extern double pauseSeconds;
extern double totalPauseSeconds;
extern double seekAccumulatedSeconds;
extern FileSystemEntry *library;

int printPlayer(SongData *songdata, double elapsedSeconds, AppSettings *settings, AppState *appState);

void flipNextPage(void);

void flipPrevPage(void);

void showHelp(void);

void setChosenDir(FileSystemEntry *entry);

int printAbout(SongData *songdata, UISettings *ui);

FileSystemEntry *getCurrentLibEntry(void);

FileSystemEntry *getChosenDir(void);

FileSystemEntry *getLibrary(void);

void scrollNext(void);

void scrollPrev(void);

void setCurrentAsChosenDir(void);

void toggleShowView(ViewState VIEW_TO_SHOW);

void toggleShowRadioSearch(void);

void showTrack(void);

void freeMainDirectoryTree(AppState *state);

char *getLibraryFilePath(void);

void resetChosenDir(void);

void switchToNextView(void);

void switchToPreviousView(void);

void resetSearchResult(void);

void resetRadioSearchResult(void);

int getChosenRow(void);

void setChosenRow(int row);

#endif
