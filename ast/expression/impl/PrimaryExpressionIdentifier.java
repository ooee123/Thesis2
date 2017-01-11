package ast.expression.impl;

import ast.expression.Expression;
import ast.expression.PrimaryExpression;
import ast.type.Type;
import com.google.common.collect.Sets;
import lombok.Data;
import lombok.NonNull;
import lombok.RequiredArgsConstructor;
import visitor.Visitor;

import java.util.Collections;
import java.util.Set;

/**
 * Created by kly04 on 9/26/16.
 */
@Data
@RequiredArgsConstructor
public class PrimaryExpressionIdentifier implements PrimaryExpression {
    @NonNull private String identifier;
    private Type type;

    public PrimaryExpressionIdentifier(String identifier, Type type) {
        this(identifier);
        this.type = type;
    }

    public String toCode() {
        return identifier;
    }

    @Override
    public Set<String> getVariables() {
        return Sets.newHashSet(identifier);
    }

    @Override
    public Set<String> getDependentVariables() {
        return Sets.newHashSet(identifier);
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return Sets.newHashSet();
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return Collections.emptySet();
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        return;
    }
}
