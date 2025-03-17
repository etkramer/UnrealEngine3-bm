// BM1
class MaterialExpressionLightingSpecularBlinnPhong extends MaterialExpression within Material
    native(Material)
    collapsecategories;

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
