# fishinator
## Features 
**Board representation**
* [Bitboards](https://www.chessprogramming.org/Bitboards) ([Little-Endian Rank-File Mapping](https://www.chessprogramming.org/Square_Mapping_Considerations#LittleEndianRankFileMapping))
* [Magic Bitboards](https://www.chessprogramming.org/Magic_Bitboards) ([Plain](https://www.chessprogramming.org/Magic_Bitboards#Plain))

**Communication protocol**
* [UCI](https://en.wikipedia.org/wiki/Universal_Chess_Interface)

**Search**
* [Alpha-Beta](https://www.chessprogramming.org/Alpha-Beta)
* [Quiescence](https://www.chessprogramming.org/Quiescence_Search)
* [Move Ordering](https://www.chessprogramming.org/Move_Ordering)
    * MVV-LVA
    * Killer Heuristic
    * History Heuristic

**Evaluation**
* [testing purposes] [PeSTO's Evaluation Function](https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function)

## How to
### Compile and run
```bash
make && ./fishinator
```

### Perft
[More about Perft](https://www.chessprogramming.org/Perft).

```bash
position [fen <fenstring> | startpos] moves <move1> .... <movei>
go perft <depth>
```

**Example**: 
```bash
position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -
go perft 5
```

### GUI
fishinator is compatible with [UCI GUIs](https://www.chessprogramming.org/UCI#GUIs).