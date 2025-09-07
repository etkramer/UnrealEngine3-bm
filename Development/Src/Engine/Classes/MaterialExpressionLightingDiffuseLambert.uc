// BM1
class MaterialExpressionLightingDiffuseLambert extends MaterialExpression
	native(Material);

cpptext
{
	virtual INT Compile(FMaterialCompiler* Compiler);
	virtual FString GetCaption() const;
}

var ExpressionInput Power;
var ExpressionInput PowerMin;
var ExpressionInput PowerMax;
var ExpressionInput FresnelStraight;
var ExpressionInput FresnelGlancing;
var ExpressionInput FresnelPower;
var ExpressionInput Colour;
var() Texture Lookup;
