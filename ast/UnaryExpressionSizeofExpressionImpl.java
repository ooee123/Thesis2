package ast;

import ast.type.Type;
import lombok.Value;

import java.util.Collections;
import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class UnaryExpressionSizeofExpressionImpl implements UnaryExpression {
    private UnaryExpression unaryExpression;

    @Override
    public Set<String> getLValues() {
        return Collections.emptySet();
    }

    public String toCode() {
        return "sizeof " + unaryExpression.toCode();
    }

    @Override
    public Set<String> getVariables() {
        return unaryExpression.getVariables();
    }
}
