package ast.expression.impl;

import ast.expression.Expression;
import ast.expression.PrimaryExpression;
import com.google.common.collect.Sets;
import lombok.Value;
import visitor.Visitor;

import java.util.Collections;
import java.util.Set;

/**
 * Created by kly04 on 9/26/16.
 */
@Value
public class PrimaryExpressionConstant implements PrimaryExpression {
    private Object value;

    public PrimaryExpressionConstant(long l) {
        value = l;
    }

    public PrimaryExpressionConstant(double d) {
        value = d;
    }

    public PrimaryExpressionConstant(char c) {
        value = c;
    }

    public PrimaryExpressionConstant(String s) {
        value = s;
    }

    public String toCode() {
        if (value instanceof Character) {
            return "'" + value.toString() + "'";
        }
        if (value instanceof String) {
            return "\"" + value.toString() + "\"";
        }
        return value.toString();
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
