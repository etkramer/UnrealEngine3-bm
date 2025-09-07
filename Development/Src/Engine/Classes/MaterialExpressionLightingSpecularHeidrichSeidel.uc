// BM1
class MaterialExpressionLightingSpecularHeidrichSeidel extends MaterialExpression
	native(Material);

cpptext
{
	virtual INT Compile(FMaterialCompiler* Compiler);
	virtual FString GetCaption() const;
}

var ExpressionInput Power;
var ExpressionInput PowerMin;
var ExpressionInput PowerMax;
var ExpressionInput FresnelPower;
var ExpressionInput FresnelStraight;
var ExpressionInput FresnelGlancing;
var ExpressionInput FalloffPower;
var ExpressionInput FalloffStraight;
var ExpressionInput FalloffGlancing;
var ExpressionInput Colour;
var() Texture Lookup;
var() bool UseAccurateTangent;
