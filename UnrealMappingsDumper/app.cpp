#include "pch.h"

#include "app.h"

IDumper* CreateAppInstance(EUnrealVersion Version)
{
	switch (Version)
	{
	case EUnrealVersion::UE5_01:
		return new Dumper<Engine_UE5_01>();
		break;
	default:
		return new Dumper<DefaultEngine>();
		break;
	}
}