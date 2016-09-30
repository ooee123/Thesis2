package ast;

import lombok.Value;

import java.util.Collections;
import java.util.List;
import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class PostfixExpressionInvocationImpl implements PostfixExpression {
    private PostfixExpression postfixExpression;
    private List<AssignmentExpression> arguments;

    @Override
    public Set<String> getLValues() {
        return Collections.emptySet();
    }
}
