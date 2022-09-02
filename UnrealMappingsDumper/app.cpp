#include "pch.h"

#include "app.h"

IDumper* CreateAppInstance(EUnrealVersion Version)
{
	switch (Version)
	{
	case EUnrealVersion::UE5:
	case EUnrealVersion::UE5_01:
		return new Dumper<Engine_UE5>();
		break;
	default:
		return new Dumper<DefaultEngine<>>();
		break;
	}
}