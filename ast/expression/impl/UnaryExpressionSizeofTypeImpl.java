package ast.expression.impl;

import ast.expression.Expression;
import ast.expression.UnaryExpression;
import ast.expression.impl.PostfixExpressionInvocationImpl;
import ast.type.Type;
import com.google.common.collect.Sets;
import lombok.Value;
import visitor.Visitor;

import java.util.Collections;
import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class UnaryExpressionSizeofTypeImpl implements UnaryExpression {
    private Type type;

    public String toCode() {
        return "sizeof(" + type.toCode() + ")";
    }

    @Override
    public Set<String> getVariables() {
        return Collections.emptySet();
    }

    @Override
    public Set<String> getDependentVariables() {
        return Sets.newHashSet();
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return Sets.newHashSet();
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return Sets.newIdentityHashSet();
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        return;
    }
}
