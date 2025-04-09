# Hello the witness

> This project is just for programming practice.  
> [Buy The Witness on Steam](https://store.steampowered.com/app/210970/The_Witness/)

*This project is in progress.*  
And most code from -> [itch normalexisting](https://normalexisting.itch.io/witness-clone)

## Quick start

### macOS Users

Dependence

```bash
brew install raylib
```

```bash
git clone https://github.com/qiekn/the-witness.git
cmake -B build && make -j$(nproc) -C build
./build/game
```

`compile_commands.json` for neovim lsp config:

```bash
cd path/to/your/project
ls -s build/compile_commands.json ./compile_commands.json
```

**Credit**:  WW92030-STORAGE/WW92030/NORMALEXISTING (Original Author‘s Different IDs)
