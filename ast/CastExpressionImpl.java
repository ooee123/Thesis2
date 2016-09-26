package ast;

import ast.type.Type;
import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class CastExpressionImpl implements CastExpression {
    private Type type;
    private CastExpression castExpression;
}
