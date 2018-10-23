package profiler.dodola.lib;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;


/**
 * Object stands for a Java Method, may be a constructor or a method.
 */
public class ArtMethod {

    private static final String TAG = "ArtMethod";

    /**
     * The address of the Art method. this is not the real memory address of the java.lang.reflect.Method
     * But the address used by VM which stand for the Java method.
     * generally, it was the address of art::mirror::ArtMethod. @{link #objectAddress}
     */
    private long address;

    /**
     * The address of the java method(Java Object's address), which may be move from gc.
     */
    private long objectAddress;

    /**
     * the origin object if this is a constructor
     */
    private Constructor constructor;

    /**
     * the origin object if this is a method;
     */
    private Method method;

    /**
     * the origin ArtMethod if this method is a backup of someone, null when this is not backup
     */
    private ArtMethod origin;

    /**
     * The size of ArtMethod, usually the java part of ArtMethod may not stand for the whole one
     * may be some native field is placed in the end of header.
     */
    private static int artMethodSize = -1;

    private ArtMethod(Constructor constructor) {
        if (constructor == null) {
            throw new IllegalArgumentException("constructor can not be null");
        }
        this.constructor = constructor;
        init();
    }

    private ArtMethod(Method method) {
        if (method == null) {
            throw new IllegalArgumentException("method can not be null");
        }
        this.method = method;
        init();
    }

    private void init() {
        if (constructor != null) {
            address = InnerHooker.getMethodAddress(constructor);
            objectAddress = Unsafe.getObjectAddress(constructor);
        } else {
            address = InnerHooker.getMethodAddress(method);
            objectAddress = Unsafe.getObjectAddress(method);
        }
    }

    public static ArtMethod of(Method method) {
        return new ArtMethod(method);
    }

    public static ArtMethod of(Constructor constructor) {
        return new ArtMethod(constructor);
    }


    /**
     * @return is method/constructor accessible
     */
    public boolean isAccessible() {
        if (constructor != null) {
            return constructor.isAccessible();
        } else {
            return method.isAccessible();
        }
    }

    /**
     * make the constructor or method accessible
     *
     * @param accessible accessible
     */
    public void setAccessible(boolean accessible) {
        if (constructor != null) {
            constructor.setAccessible(accessible);
        } else {
            method.setAccessible(accessible);
        }
    }

    /**
     * get the origin method's name
     *
     * @return constructor name of method name
     */
    public String getName() {
        if (constructor != null) {
            return constructor.getName();
        } else {
            return method.getName();
        }
    }

    public Class<?> getDeclaringClass() {
        if (constructor != null) {
            return constructor.getDeclaringClass();
        } else {
            return method.getDeclaringClass();
        }
    }

    public int getModifiers() {
        if (constructor != null) {
            return constructor.getModifiers();
        } else {
            return method.getModifiers();
        }
    }

    /**
     * get the parameter type of origin method/constructor
     *
     * @return the parameter types.
     */
    public Class<?>[] getParameterTypes() {
        if (constructor != null) {
            return constructor.getParameterTypes();
        } else {
            return method.getParameterTypes();
        }
    }

    /**
     * get the return type of origin method/constructor
     *
     * @return the return type, if it is a constructor, return Object.class
     */
    public Class<?> getReturnType() {
        if (constructor != null) {
            return Object.class;
        } else {
            return method.getReturnType();
        }
    }

    /**
     * get the exception declared by the method/constructor
     *
     * @return the array of declared exception.
     */
    public Class<?>[] getExceptionTypes() {
        if (constructor != null) {
            return constructor.getExceptionTypes();
        } else {
            return method.getExceptionTypes();
        }
    }

    public String toGenericString() {
        if (constructor != null) {
            return constructor.toGenericString();
        } else {
            return method.toGenericString();
        }
    }

    /**
     * @return the origin method/constructor
     */
    public Object getExecutable() {
        if (constructor != null) {
            return constructor;
        } else {
            return method;
        }
    }

    /**
     * get the memory address of the inner constructor/method
     *
     * @return the method address, in general, it was the pointer of art::mirror::ArtMethod
     */
    public long getAddress() {
        return address;
    }

    /**
     * get the unique identifier of the constructor/method
     *
     * @return the method identifier
     */
    public String getIdentifier() {
        // Can we use address, may gc move it??
        return String.valueOf(getAddress());
    }

    /**
     * force set the private flag of the method.
     */
    public void makePrivate() {
        int accessFlags = getAccessFlags();
        accessFlags &= ~Modifier.PUBLIC;
        accessFlags |= Modifier.PRIVATE;
        setAccessFlags(accessFlags);
    }


    public long getEntryPointFromQuickCompiledCode() {
        return Offset.read(address, Offset.ART_QUICK_CODE_OFFSET);
    }

    public void setEntryPointFromQuickCompiledCode(long pointer_entry_point_from_quick_compiled_code) {
        Offset.write(address, Offset.ART_QUICK_CODE_OFFSET, pointer_entry_point_from_quick_compiled_code);
    }

    public int getAccessFlags() {
        return (int) Offset.read(address, Offset.ART_ACCESS_FLAG_OFFSET);
    }

    public void setAccessFlags(int newFlags) {
        Offset.write(address, Offset.ART_ACCESS_FLAG_OFFSET, newFlags);
    }

    public void setEntryPointFromJni(long entryPointFromJni) {
        Offset.write(address, Offset.ART_JNI_ENTRY_OFFSET, entryPointFromJni);
    }

    public long getEntryPointFromJni() {
        return Offset.read(address, Offset.ART_JNI_ENTRY_OFFSET);
    }


}