// BM1
class MaterialExpressionLightingDiffuseLambert extends MaterialExpression within Material
    native(Material)
    collapsecategories;

var ExpressionInput Power;
var ExpressionInput PowerMin;
var ExpressionInput PowerMax;
var ExpressionInput FresnelStraight;
var ExpressionInput FresnelGlancing;
var ExpressionInput FresnelPower;
var ExpressionInput Colour;
var() Texture Lookup;
