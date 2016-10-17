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
    public Set<String> getChangedVariables() {
        return castExpression.getChangedVariables();
    }

    public String toCode() {
        return "(" + type.toString() + ")" + castExpression.toCode();
    }

    @Override
    public Set<String> getDependentVariables() {
        return castExpression.getDependentVariables();
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return castExpression.getInvocations();
    }

    @Override
    public Set<String> getVariables() {
        return castExpression.getVariables();
    }
}
