package visitor;

import ast.*;
import lombok.Value;

import java.util.ArrayList;
import java.util.Collection;

/**
 * Created by ooee on 10/22/16.
 */
@Value
public class GetInvocationsVisitor implements Visitor<Expression> {
    private Collection<PostfixExpressionInvocationImpl> invocations;

    public GetInvocationsVisitor() {
        invocations = new ArrayList<>();
    }

    @Override
    public void visit(Expression expression) {
        if (expression instanceof PostfixExpressionInvocationImpl) {
            invocations.add(((PostfixExpressionInvocationImpl) expression));
        }
        expression.visitNestedExpressions(this);
    }
}
