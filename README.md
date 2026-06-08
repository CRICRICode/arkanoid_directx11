# Arkanoid DirectX 11

Versione semplificata di Arkanoid sviluppata in C++ con DirectX 11 per il corso di Graphics.

Il progetto realizza un singolo livello giocabile con racchetta, pallina, mattoncini colorati, collisioni 2D e reset automatico del livello quando tutti i mattoncini vengono distrutti.

## Funzionalita

- Un solo livello di gioco.
- Mattoncini distrutti al primo colpo.
- Colori dei mattoncini generati casualmente, con almeno cinque colori diversi a ogni reset.
- Reset del livello quando tutti i mattoncini sono stati distrutti.
- Una sola vita: se la racchetta manca la pallina, la partita termina.
- Restart manuale dopo il game over con `Invio`.
- Movimento della racchetta tramite tastiera.
- Modificatori opzionali rilasciati casualmente da alcuni mattoncini.
- Rendering 2D tramite DirectX 11 e shader HLSL.

## Controlli

| Tasto | Azione |
| --- | --- |
| `Freccia sinistra` / `A` | Muove la racchetta a sinistra |
| `Freccia destra` / `D` | Muove la racchetta a destra |
| `Invio` | Riavvia la partita dopo il game over |
| `Esc` | Chiude l'applicazione |

## Modificatori

Alcuni mattoncini possono rilasciare un modificatore temporaneo:

- pallina piu grande;
- controlli invertiti e scena ruotata;
- racchetta piu lunga;
- velocita aumentata.

I modificatori sono feature opzionali rispetto alla traccia base.

## Requisiti

- Windows.
- Visual Studio 2022.
- Toolset MSVC v143.
- Windows SDK 10.
- Supporto DirectX 11.

## Build

Aprire la soluzione:

```text
directx11_1.sln
```

Configurazione consigliata:

```text
Debug | x64
```

Da terminale, con MSBuild disponibile:

```powershell
MSBuild.exe directx11_1.sln /p:Configuration=Debug /p:Platform=x64 /m
```

L'eseguibile viene generato in:

```text
x64\Debug\directx11_1.exe
```

## Struttura del progetto

- `applicationclass.*`: logica di gioco, stato, collisioni e rendering degli oggetti.
- `d3dclass.*`: inizializzazione e gestione delle risorse DirectX 11.
- `modelclass.*`: geometria riutilizzabile per rettangoli e cerchi.
- `colorshaderclass.*`: compilazione shader e passaggio matrici alla GPU.
- `inputclass.*`: stato della tastiera.
- `systemclass.*`: finestra Win32 e ciclo principale.
- `color.vs` / `color.ps`: shader HLSL.

## Note

Il progetto e pensato come esercizio didattico: privilegia una struttura semplice e leggibile, mantenendo separate la logica di gioco, la gestione DirectX e il ciclo applicativo.
