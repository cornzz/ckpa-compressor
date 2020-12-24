# Technische Dokumentation des Kompressors

## Gliederung

1. Systemvorraussetzungen
2. Verwendung
3. Bedienung
4. Systemarchitektur
	* Projektstruktur
		* Verzeichnisstruktur
	* Komponenten und Funktionen
		* FF_Meters
		* PluginProcessor
		* PluginEditor
		* Visualizer
		
## Systemvorraussetzungen

### Plattformabhängige Debug-Anforderungen

* macOS	
	* 10.9 oder höher
	* Xcode 7.3 oder höher
* Windows
	* 8.1 oder höher
	* Visual Studio 2015 oder höher
	* 64 Bit	
* Linux
	* Ubuntu 16.04.3 LTS
	* Makefile

### Allgemeine Debug-Anforderungen

* JUCE 5
* Windows SDK 10.0.18362.1

## Einrichtung (Windows)

### zur Nutzung

1. [CKPA_Kompressor.vst3](https://github.com/cornzz/ckpa-compressor/releases/latest) herunterladen und entpacken
2. CKPA_Kompressor.vst3 in beliebiger DAW / beliebigem VST3 Plugin Host öffnen

### zur Entwicklung

* Visual Studio mit Paketen für die C++ Entwicklung installieren
* JUCE herunterladen
* Projekt Repository beziehen:
	* SSH: git@github.com:cornzz/ckpa-compressor.git
	* HTTPS: https://github.com/cornzz/ckpa-compressor.git
* CKPA_Compressor.jucer im Projucer öffnen, Visual Studio Button anklicken
* "VST3 Plugin" als Startprojekt festlegen
* Einstellungen von "VST3 Plugin" öffnen, unter "Debugging" die DAW / VST3 Plugin Host Executable als Befehl auswählen
* Mit F5 den Debugger starten

## Bedienung

Das Plugin besteht aus 3 Levels, die jeweils in einem Tab angezeigt werden.
Um in ein bestimmtes Level zu wechseln, klicke den jeweiligen Tab mit der entsprechenden Level Beschriftung an.
Für eine ausführliche Beschreibung der Levels in der wissenschaftlichen Dokumentation nachschauen.

### Level 1

In diesem Level können die Parameter Threshold, Ratio, Attack, Release und Makeup Gain durch Bewegen des jeweiligen Schiebereglers beeinflusst werden. 

### Level 2

In diesem Level können nur Threshold, Ratio und Makeu Gain beeinflusst werden.
Das ist durch Bewegen des jeweiligen Labels möglich (intern).

### Level 3

In diesem Level werden Threshold, Ratio und Makeup Gain beinflusst durch
Bewegen des Schiebereglers auf dem Rand des Kreises.

## Systemarchitektur

### Projektstruktur

#### Verzeichnisstruktur

* FXPlugin-Dynamics-SoSe20
	* Source
		* PluginProcessor: Kern des Plugins; Kompressionsalgorithmus, Initialisierung und Verwaltung der Parameter
		* PluginParameters: Parameterstruktur, Parameterverwaltung
		* PluginEditor: Plugin Hauptfenster, Instanziierung der einzelnen Levels
		* LevelXEditor: Darstellung der einzelnen Levels
		* Visualiser: Visualierung des Signals
		* MainTabbedComponent: Tab-Funktion 
		* ff_meters (Ordner): Eingebundene Bibliothek zur Darstellung der Pegelmesser

### Komponenten und Funktionen

#### Plugin Processor

Diese Komponente berechnet alle Parameter: "Threshold, Ratio, Attack, Release und Makeup Gain". 
Außerdem existiert noch ein "Bypass" - Parameter, wodurch keine Veränderungen am Signal ausgeführt werden.
Der Parameter "Compression" wurde für das 3. Level eingeführt, um Threshold, Ratio und Makeup Gain in
einem Wert zuvereinen.

#### Plugin Editor

Diese Komponente   
Dafür besitzt sie ein TabbedComponent Objekt, welches mit Instanzen der Klassen LevelXEditor (X = 1, 2, 3) initialisiert wird.
Die Levels sind von einander unabhängige Objekte, initialisieren ihre eigenen Komponenten und haben eine eigene Referenz zum PluginProcessor.

#### Plugin Parameter Manager

Der Plugin Paramter Manager verwaltet die Parameter Daten durch
Speicherung und Verknüpfung mit den dazugehörigen User Interface Elementen.

#### Visualiser

Die Komponente übernimmt die Darstellung des Signals in Waveform, welche in Level 2 verwendet wird.
