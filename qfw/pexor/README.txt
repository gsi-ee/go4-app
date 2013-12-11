//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at CSEE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum für Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------


---------------------------------------------
// Readout of QFW beam diagnostic board via PEXOR family optical receiver.
// V 0.9 11-Dec-2013
// Joern Adamczewski-Musch, CSEE, GSI Darmstadt
// j.adamczewski@gsi.de
---------------------------------------------



Note: English description currently not available, sorry. 
We will translate the German explanations as soon as possible JAM

##############################################################################
---------------------------------------------------------------------------
Usage of this software for QFW test:

1) Ist installiert auf sdpc095.gsi.de als user mbsdaq im Verzeichnis
~/go4_qfw/hd/pexor. Das zugehörige Repository (zur Installation auf beliebigem Rechner mit Go4 framework) ist erreichbar unter
https://subversion.gsi.de/go4/app/qfw/pexor

***********************************************************************
2) Starten:
Nach ". go4login"
mit "cd /LynxOS/mbsusr/mbsdaq/go4_qfw/hd/pexor" ins Arbeitsverzeichnis wechseln.

Dort gibt ein hotstart script das als default das lmd file
/data/HIT_run12_0001.lmd öffnet:
"go4 hit" startet dieses. (Die Analyse ist hier am Anfang gestoppt, d.h. nochmal auf grünen Pfeil drücken!)

*********************************************************************** 

3) Setup:

Die Verkabelung der qfw boards per sfp zum pexor wird in der Klasse
TQFWRawParam über die Arrays fBoardID[sfp][dev] eingestellt.
Dabei ist
    sfp: sfp Strang am Pexor (0,..,3)
    dev: device nummer an diesem Strang (0,...,255?)
    fBoardID: eine (hier willkürliche) eindeutige GeräteID des QFW boards. Für den HIT Aufbau sind hier die ids 10,11, 12 gewählt. Später sollten diese Nummern irgendwie auf der Board hardware zu finden sein...
Funktion TQFWRawParam::InitBoardMapping() in TQFWRawParam.cxx initialisiert dies mit dem Aufbau am HIT

Die Verkabelung der qfw boards zu den grids/faraday cups etc ist in der Klasse TQFWProfileParam eingestellt mit Parameter Arrays über einen laufenden index aller Profilgitter (grid) und Gitterdrähten (wire):

 fNumGrids - Anzahl aller Gitter im Aufbau

 fGridDeviceID[grid] - Unique id des Gitters am laufenden index grid. Diese ist zunächst willkürlich, sollte später aber auf der hardware wiederzufinden sein.


 fGridBoardID_X[grid][wire] -
    Unique id des qfw boards (s.o.) an Gitter grid, Draht wire in X Richtung

 fGridBoardID_Y[grid][wire] -
Unique id des qfw boards an Gitter grid, Draht wire in Y Richtung

 fGridChannel_X[grid][wire] - Nummer des qfw board Kanals (0...32) für Gitter grid, Draht wire in X

 fGridChannel_Y[grid][wire] -  Nummer des qfw board Kanals (0...32) für Gitter grid, Draht wire in Y


Neben den Strahlprofilgittern können "Cup" Objekte konfiguriert werden, d.h. irgendwie segmentierte Ladungsmessungsplatten. Dies geschieht mit Parametern über indices der Segmentierten Einheiten (cup) und der Segmentnummer (seg)

fNumCups - Anzahl aller "Segmentierten Cups"

fCupDeviceID[cup] -
    Unique id des Cup Devices für index cup
fCupBoardID[cup][seg]-
    Unique id des qfw boards an Cup cup, Segment seg
 fCupChannel[cup][seg] -
      Nummer des qfw board Kanals (0...32) an Cup cup, Segment seg

Negative Werte für diese Arrays deaktiviert den Entsprechenden Kanal!

Die Konfiguration der Time Slices passiert automatisch anhand der eingelesenen Daten!

Für die HIT Strahlzeit gibt es hier ein Gitter mit willkürlicher ID 42
(verbunden mit boards 10 und 11) und ein "Cup" mit id 66 und 2 Segmenten, das sind die schräg segmentierten Kondensatorplatten (?)


*************************************************************************
4) Histogramm Displays:

Die Go4 Analyse hat nun 2 Stufen (Analysis Steps):
Die erste Stufe packt die Rohdaten von der DAQ aus und zeigt die QFW Board Kanäle. Deren Histogramme befinden sich unter Histograms/Board%d
(also in unserem Fall Board10, Board11, Board12 für die von mir willkürlich vergebenen "unique ids").

Die zweite Stufe mappt die qfw Kanäle entsprechend der oben beschriebenen Zuweisungen auf die messenden Geräte, also Gitter oder segmentierte Platten/Cups. Deren Histogramme sind unter
Histograms/Beam/Grid42 bzw Histograms/Beam/Cup66 zu finden.

Für jedes board/grid/cup gibt es Unterordner Loop0,Loop1,Loop2, die spezifische Histogramme für den jeweiligen Zeitloop enthalten.


Besondere Histogramme für grids:
Meanpos_G42: Statistik des mean values über alle x/y Profile. Das sollte eine Art Strahlpositon in x/y wiedergeben

RMS_X_G42: Statistik des rms values über alle x/y Profile in x. Das sollte eine Art rms Strahldurchmesser in x ergeben

RMS_Y_G42: Statistik des rms values über alle x/y Profile in x. Das sollte eine Art rms Strahldurchmesser in x ergeben


Besondere Histogramme für segmentierte cups:
Cup66/Loop1/SegmentRatio_C66_L1_Si : Verhältnis der Ladung auf Segmenti zur Ladung auf allen Segmenten (für dieses Event) dito für andere Timeloops

Cup66/Loop1/SegmentRatioSum_C66_L1_Si : Verhältnis der Ladung auf Segmenti zur Ladung auf allen Segmenten (akkumuliert für alle Events). dito für andere Timeloops

Dies kann zur Positionsbestimmung des Strahls relativ zu den Segmenten benutzt werden.


Pictures:
Mehrere Histogramme für ein Device sind in "Go4 Pictures" zusammengefasst:
z.B.
Pictures/Board10/QFW_Rawscalers_Brd10 - alle scaler für board 10 Kanäle

Pictures/Beam/Grid42/Beam Display Grid42
- Projektionen des "Strahlprofils" in x/y

Pictures/Beam/Grid42/Beam RMS Grid42
- RMS "Strahldurchmesser" (s.o.) in x und y


**************************************************************************
5) Besondere Analysemodi:

--------------------------------------------------------------------------
A) Messung des Offsets und die Korrektur
Interaktiv einstellbar im Parameter Parameters/QFWProfileParam (Doppelklick auf Go4 browser öffnet Parameter Editor).

Zum Starten der Offset Messung "fMeasureBackground" im Parameter Editor auf 1 (true) setzen, dann Parameter mit "Return" und Pfeil nach links Knopf" aktivieren und lmd file mit Offset einlesen. Sobald Offset Messung beendet ist "fMeasureBackground" wieder auf 0 setzen  und lmd file mit beamdaten einlesen. Dann sollte der offset in der Darstellung der _grids_ und der _cups_ abgezogen werden (! die Board histogramme werden nie korrigiert!)

Untergrundkorrektur kann zusätzlich mit
"fCorrectBackground" ein und ausgeschaltet werden.

Zur Kontrolle der gerade zur Korrektur benutzten Offsets dienen Histogramme
"Beam/Grid42/Loop 1/Profile_X_Time_Offset_G42_L1" (analog für andere timeloops und grid ids)
bzw.
Beam/Cup66/Loop 1/Scaler_Time_Offset_C42_L1 (dito)

---------------------------------------------------------------------------
B) Slow motion:
Interaktiv einstellbar im Parameter Parameters/QFWProfileParam (Doppelklick auf Go4 browser öffnet Parameter Editor).
"fSlowMotionStart" auf 0: Normaler Betrieb
Setzen von
"fSlowMotionStart" auf eine Eventnummer aktiviert Zeitlupenmodus für alle Events mit größerer Laufender Nummer, d.h. nach jeweils einem Event wird die Analyse angehalten und man kann nach Refresh des Go4 Displays (F5) sich den aktuellen Trace ansehen. Drücken des Startknopfes (grüner Pfeil oder CTRL-S) holt dann das nächste Event, usw.
Achtung: Laufende Nummer ist die vom QFW in den Daten eingeschriebene Sequenznummer, nicht die Nummer der Go4 Events!

------------------------------------------------------------------------
C) Überspringen von "schlechten Events" bei freilaufender DAQ

Ziel: Anhand der Ladung auf ausgewählten Kanälen soll entschieden werden, ob es sich um ein Strahlevent (Strahl voll im Setup), ein Untergrundevent (kein Strahl), oder ein "schlechtes Event" (Strahl halb drin) handelt

Einstellbar im Parameter Parameters/QFWRawParam:

fSelectTriggerEvents; // switch triggering of good and background events for free running daq

fTriggerHighThreshold; // selects good event if integral of trigger channel counts is above

fTriggerLowThreshold; // selects background events if integral of trigger channel counts is below

fTriggerBoardID; // id of board that contains trigger channels

fTriggerFirstChannel; // lower boundary of trigger channel region

fTriggerLastChannel; // upper boundary of trigger channel region

Welche QFW Kanäle für die Triggerentscheidung aufsummiert werden, wird mit fTriggerBoardID (unique id des QFW boards, siehe oben) und einem
Kanalbereich
fTriggerFirstChannel <= qfwchannel <= fTriggerLastChannel
eingestellt.
Für das hit setup ist hier der default eingestellt
in TQFWRawParam::InitBoardMapping() -
fTriggerBoardID=12, fTriggerFirstChannel=0, fTriggerLastChannel=1


Durchgelassen werden hier nur die beam events (Summe der Ladung ist oberhalb von fTriggerHighThreshold) und die background events
(Summe der Ladung ist unterhalb von fTriggerLowThreshold). Dazwischenliegende halbe Events werden übersprungen. Man erhält für jedes übersprungene Event eine Meldung im Analyseterminal:
GO4-*> Skip event of seqnr 9327 with triggersum 4466225!
(mit QFW event sequenznummer und Ladungssumme der Triggerkanäle)

************************************************************************ 


JAM 11-Dec-2013