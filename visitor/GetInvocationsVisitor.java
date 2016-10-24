package visitor;

import ast.*;

import java.util.ArrayList;
import java.util.Collection;

/**
 * Created by ooee on 10/22/16.
 */
public class GetInvocationsVisitor implements Visitor<PostfixExpressionInvocationImpl, Expression> {
    @Override
    public Collection<PostfixExpressionInvocationImpl> visit(Expression expression) {
        return expression.getInvocations();
    }
}
