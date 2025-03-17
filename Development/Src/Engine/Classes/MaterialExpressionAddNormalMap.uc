// BM1
class MaterialExpressionAddNormalMap extends MaterialExpression within Material
    native(Material)
    collapsecategories;

var() Texture Texture;
var() float AddU;
var() float AddV;
var() float MultiplyU;
var() float MultiplyV;
var() float TileU;
var() float TileV;
var ExpressionInput Scale;
var ExpressionInput Base;

defaultproperties
{
    bRealtimePreview=true
}
