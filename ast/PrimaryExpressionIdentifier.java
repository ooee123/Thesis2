package ast;

import com.google.common.collect.Sets;
import lombok.Value;

import java.util.Set;

/**
 * Created by kly04 on 9/26/16.
 */
@Value
public class PrimaryExpressionIdentifier implements PrimaryExpression {
    private String identifier;

    @Override
    public Set<String> getLValues() {
        return Sets.newHashSet(identifier);
    }

    public String toCode() {
        return identifier;
    }
}
