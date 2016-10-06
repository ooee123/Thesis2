package ast;

import ast.type.Type;
import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class UnaryExpressionSizeofExpressionImpl implements UnaryExpression {
    private UnaryExpression unaryExpression;

    @Override
    public String toCode() {
        return "sizeof " + unaryExpression.toCode();
    }
}
