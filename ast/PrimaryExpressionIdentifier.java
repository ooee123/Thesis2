package ast;

import lombok.Value;

/**
 * Created by kly04 on 9/26/16.
 */
@Value
public class PrimaryExpressionIdentifier implements PrimaryExpression {
    private String identifier;
}
