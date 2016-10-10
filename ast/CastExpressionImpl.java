package ast;

import ast.type.Type;
import lombok.Value;

import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class CastExpressionImpl implements CastExpression {
    private Type type;
    private CastExpression castExpression;

    @Override
    public Set<String> getLValues() {
        return castExpression.getLValues();
    }

    public String toCode() {
        return "(" + type.toString() + ")" + castExpression.toCode();
    }

    @Override
    public Set<String> getVariables() {
        return castExpression.getVariables();
    }
}
