/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTDmgType_Slime extends UTDamageType
	abstract;


static function DoCustomDamageEffects(UTPawn ThePawn, class<UTDamageType> TheDamageType, const out TraceHitInfo HitInfo, vector HitLocation)
{
	local int i, Num;
	local MaterialInstanceConstant OldMIC, NewMIC;
	local int ValueIndex;
	local bool bFoundDiffuse;
	local Texture DiffuseTexture;


	if (ThePawn.CurrCharClassInfo != None && ThePawn.CurrCharClassInfo.default.BioDeathMICParent != None)
	{
		Num = ThePawn.Mesh.GetNumElements();
		for (i = 0; i < Num; i++)
		{
			OldMIC = MaterialInstanceConstant(ThePawn.Mesh.GetMaterial(i));
			if (OldMIC != None)
			{
				// look up the chain until we find an MIC with the texture parameters that we need
				bFoundDiffuse = FALSE;
				//bFoundDiffuse = OldMic.GetTextureParameterValue( 'Char_Diffuse', DiffuseTexture );
				for( ValueIndex = 0;ValueIndex < OldMic.TextureParameterValues.Length;ValueIndex++)
				{
					if( OldMic.TextureParameterValues[ValueIndex].ParameterName == 'Char_Diffuse' )
					{
						OldMic.GetTextureParameterValue( 'Char_Diffuse', DiffuseTexture );
						bFoundDiffuse = TRUE;
						break;
					}
				}

				while( bFoundDiffuse != TRUE )
				{
					OldMic = MaterialInstanceConstant(OldMic.Parent);
					for( ValueIndex = 0;ValueIndex < OldMic.TextureParameterValues.Length;ValueIndex++)
					{
						if( OldMic.TextureParameterValues[ValueIndex].ParameterName == 'Char_Diffuse' )
						{
							OldMic.GetTextureParameterValue( 'Char_Diffuse', DiffuseTexture );
							bFoundDiffuse = TRUE;
							break;
						}
					}
				}

				OldMIC = MaterialInstanceConstant(ThePawn.Mesh.GetMaterial(i));
				OldMIC.SetTextureParameterValue( 'Char_Diffuse', DiffuseTexture );

				// duplicate the material (copying its parameter values)
				// then set the parent to the bio material
				NewMIC = new(ThePawn) OldMIC.Class(OldMic);
				NewMIC.SetParent(ThePawn.CurrCharClassInfo.default.BioDeathMICParent);
				ThePawn.Mesh.SetMaterial(i, NewMIC);
			}
		}
		ThePawn.Mesh.AttachComponent(ThePawn.BioBurnAway, ThePawn.TorsoBoneName);
		ThePawn.BioBurnAway.ActivateSystem();
		ThePawn.bKilledByBio = TRUE;
		ThePawn.bGibbed=TRUE; // this makes it so you can't then switch to a "gibbing" weapon and get chunks
		ThePawn.UpdateShadowSettings( FALSE );
	}
}



defaultproperties
{
	DamageBodyMatColor=(R=0,G=50)
	DamageOverlayTime=0.3
	DeathOverlayTime=0.6
	bUseTearOffMomentum=false
	bCausedByWorld=true

	bUseDamageBasedDeathEffects=true
}
