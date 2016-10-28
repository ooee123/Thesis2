package ast;

import ast.type.Type;
import com.google.common.collect.Lists;
import com.google.common.collect.Sets;
import lombok.Value;
import visitor.Visitor;

import java.util.Collections;
import java.util.List;
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
        return Sets.newHashSet();
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return Sets.newHashSet();
    }

    @Override
    public List<PostfixExpressionInvocationImpl> getInvocations() {
        return Lists.newArrayList();
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        return;
    }
}
