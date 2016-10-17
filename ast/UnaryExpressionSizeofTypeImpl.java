package ast;

import ast.type.Type;
import lombok.Value;

import java.util.Collections;
import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class UnaryExpressionSizeofTypeImpl implements UnaryExpression {
    private Type type;

    public String toCode() {
        return "sizeof(" + type.toString() + ")";
    }

    @Override
    public Set<String> getVariables() {
        return Collections.emptySet();
    }

    @Override
    public Set<String> getDependentVariables() {
        return Collections.emptySet();
    }

    @Override
    public Set<String> getChangedVariables() {
        return Collections.emptySet();
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return Collections.emptySet();
    }
}
