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

    @Override
    public Set<String> getLValues() {
        return Collections.emptySet();
    }

    public String toCode() {
        return "sizeof(" + type.toString() + ")";
    }
}
