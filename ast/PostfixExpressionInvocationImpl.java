package ast;

import lombok.Value;

import java.util.List;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class PostfixExpressionInvocationImpl implements PostfixExpression {
    private PostfixExpression postfixExpression;
    private List<AssignmentExpression> arguments;
}
