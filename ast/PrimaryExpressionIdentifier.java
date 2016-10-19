package ast;

import com.google.common.collect.Sets;
import lombok.Value;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

/**
 * Created by kly04 on 9/26/16.
 */
@Value
public class PrimaryExpressionIdentifier implements PrimaryExpression {
    private String identifier;

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
        return Collections.emptySet();
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return Collections.emptySet();
    }
}
