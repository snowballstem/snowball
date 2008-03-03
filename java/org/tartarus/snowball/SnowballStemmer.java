
package org.tartarus.snowball;
import java.lang.reflect.InvocationTargetException;

public abstract class SnowballStemmer extends SnowballProgram {
    protected SnowballStemmer()
    {
	current = new StringBuffer();
	setCurrent("");
    }

    public abstract boolean stem();
};
