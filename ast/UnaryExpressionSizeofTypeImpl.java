package ast;

import ast.type.Type;
import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class UnaryExpressionSizeofTypeImpl implements UnaryExpression {
    private Type type;

    @Override
    public String toCode() {
        return "sizeof(" + type.toString() + ")";
    }
}
