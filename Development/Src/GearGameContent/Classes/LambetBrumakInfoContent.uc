class LambetBrumakInfoContent extends LambentBrumakInfo
	placeable;

var InterpCurveFloat ThreePt, TwoPt, OnePt, ZeroPt, RevOnePt, RevTwoPt;
var array<MaterialInstanceTimeVarying>	BlisterMaterialList;

simulated function PostBeginPlay()
{
	local InterpCurvePointFloat Point;

	Super.PostBeginPlay();

	// Setup curve points
	Point.InVal = 0.f;
	Point.OutVal = 0.f;
	ThreePt.Points[ThreePt.Points.length] = Point;
	Point.InVal = 1.f;
	Point.OutVal = 3.f;
	ThreePt.Points[ThreePt.Points.length] = Point;

	Point.InVal = 0.f;
	Point.OutVal = 0.f;
	TwoPt.Points[TwoPt.Points.length] = Point;
	Point.InVal = 1.f;
	Point.OutVal = 2.f;
	TwoPt.Points[TwoPt.Points.length] = Point;

	Point.InVal = 0.f;
	Point.OutVal = 0.f;
	OnePt.Points[OnePt.Points.length] = Point;
	Point.InVal = 1.f;
	Point.OutVal = 1.f;
	OnePt.Points[OnePt.Points.length] = Point;

	Point.InVal = 0.f;
	Point.OutVal = 0.f;
	ZeroPt.Points[ZeroPt.Points.length] = Point;
	Point.InVal = 1.f;
	Point.OutVal = 0.f;
	ZeroPt.Points[ZeroPt.Points.length] = Point;

	Point.InVal = 0.f;
	Point.OutVal = 1.f;
	RevOnePt.Points[RevOnePt.Points.length] = Point;
	Point.InVal = 1.f;
	Point.OutVal = 0.f;
	RevOnePt.Points[RevOnePt.Points.length] = Point;

	Point.InVal = 0.f;
	Point.OutVal = 1.f;
	RevTwoPt.Points[RevTwoPt.Points.length] = Point;
	Point.InVal = 1.f;
	Point.OutVal = 0.f;
	RevTwoPt.Points[RevTwoPt.Points.length] = Point;
	

	BlisterMaterialList.Length = 9;
	InitBlisterMaterial( 0, 1,	MaterialInterface'Locust_Brumak_Lambent.Material.MI_Lambent_Brumak_02_H' );
	InitBlisterMaterial( 1, 4,  MaterialInterface'Locust_Brumak_Lambent.Material.MI_Lambent_Brumak_05_B' );
	InitBlisterMaterial( 2, 11, MaterialInterface'Locust_Brumak_Lambent.Material.MI_Lambent_Brumak_06_B' );
	InitBlisterMaterial( 3, 12, MaterialInterface'Locust_Brumak_Lambent.Material.MI_Lambent_Brumak_07_B' );
	InitBlisterMaterial( 4, 6,  MaterialInterface'Locust_Brumak_Lambent.Material.MI_Lambent_Brumak_08_B' );
	InitBlisterMaterial( 5, 13, MaterialInterface'Locust_Brumak_Lambent.Material.MI_Lambent_Brumak_09_B' );
	InitBlisterMaterial( 6, 9,  MaterialInterface'Locust_Brumak_Lambent.Material.MI_Lambent_Brumak_10_B' );
	InitBlisterMaterial( 7, 8,  MaterialInterface'Locust_Brumak_Lambent.Material.MI_Lambent_Brumak_11_B' );
	InitBlisterMaterial( 8, 5,  MaterialInterface'Locust_Brumak_Lambent.Material.MI_Lambent_Brumak_13_B' );
}

simulated function InitBlisterMaterial( int BlisterIdx, int MatIdx, MaterialInterface MI )
{
	BlisterMaterialList[BlisterIdx] = new(Outer) class'MaterialInstanceTimeVarying';
	BlisterMaterialList[BlisterIdx].SetParent( MI );

	Mesh.SetMaterial( MatIdx, BlisterMaterialList[BlisterIdx] );
}

simulated function PlayBlisterAnim( int BlisterIdx )
{
	Super.PlayBlisterAnim( BlisterIdx );

	if( BlisterState[BlisterIdx] == EBS_Active )
	{
		BlisterMaterialList[BlisterIdx].SetScalarCurveParameterValue( 'LerpBlister', TwoPt  );
		BlisterMaterialList[BlisterIdx].SetScalarCurveParameterValue( 'LerpCrust',	 ZeroPt );
		BlisterMaterialList[BlisterIdx].SetScalarCurveParameterValue( 'LerpWound', 	 ZeroPt );
		BlisterMaterialList[BlisterIdx].SetDuration( BlisterActiveTime );
	}
	else
	if( BlisterState[BlisterIdx] == EBS_Hardened )
	{
		BlisterMaterialList[BlisterIdx].SetScalarCurveParameterValue( 'LerpBlister', RevTwoPt   );
		BlisterMaterialList[BlisterIdx].SetScalarCurveParameterValue( 'LerpCrust',	 OnePt		);
		BlisterMaterialList[BlisterIdx].SetScalarCurveParameterValue( 'LerpWound', 	 ZeroPt		);
		BlisterMaterialList[BlisterIdx].SetDuration( BlisterHardenedTime );
	}
	else
	if( BlisterState[BlisterIdx] == EBS_Wounded )
	{
		BlisterMaterialList[BlisterIdx].SetScalarCurveParameterValue( 'LerpBlister', RevTwoPt	);
		BlisterMaterialList[BlisterIdx].SetScalarCurveParameterValue( 'LerpCrust',	 ZeroPt		);
		BlisterMaterialList[BlisterIdx].SetScalarCurveParameterValue( 'LerpWound', 	 OnePt		);
		BlisterMaterialList[BlisterIdx].SetDuration( BlisterWoundedTime );
	}
}


defaultproperties
{
}