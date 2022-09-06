# UnrealMappingsDumper

This is a project which will generate mappings files for UE which will be necassary for almost all future UE5 games in order to read data from the game files.

Be sure to check the [Mappings Archive](https://github.com/OutTheShade/Unreal-Mappings-Archive) if the mappings for your game has already been dumped before. 

If you dump mappings for a game that isn't already in the [archive](https://github.com/OutTheShade/Unreal-Mappings-Archive), uploading it to the archive is encouraged.

# Usage

Either compile the DLL or get a precompiled version from [Releases](https://github.com/OutTheShade/UnrealMappingsDumper/releases). Then simply inject the DLL into the game. 

**UnrealMappingsDumper is not responsible for any bans. Take the proper precautions on anti cheat protected games.**

If all goes well, a `Mappings.usmap` file will be output to the games executing directory, which should be the same folder as the games shipping exe.

# Support

UnrealMappingsDumper is mostly targeted for UE5 games, as that's what most mappings dependent games use, but it should still work for the more recent versions of UE4.

If UnrealMappingsDumper doesn't work properly for your game, first try fixing its source code with any reversing or UE knowledge you may have. If you cannot, then feel free to open an issue for the game that needs it and its engine version. If the game is free please provide a link to download it.

# Contributing

The source code relies a lot on the power of templates in order to make overriding and changing certain types and behavior for different engine versions as easy as possible. If your contribution is engine related, just please observe the code and keep this in mind when doing so. 

## Plans

> Deducing engine version at runtime.

> More compression methods.

> More GObjects and FNameToString sigs.
